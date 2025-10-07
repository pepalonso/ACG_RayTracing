#include "whittedintegrator.h"

#include "../core/utils.h"

// WhittedIntegrator::WhittedIntegrator() :
//     color(Vector3D(1, 0, 0))
// { }

WhittedIntegrator::WhittedIntegrator(Vector3D& bgColor, int maxDepth_, float ambientTerm_) :
    Shader(bgColor), maxDepth(maxDepth_), ambientTerm(ambientTerm_)
{ }

Vector3D WhittedIntegrator::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    return computeColorRecursive(r, objList, lsList, /*depth=*/0);
}

#include <algorithm>
#include <cmath>

Vector3D WhittedIntegrator::computeColorRecursive(
    const Ray &r,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList,
    int depth) const
{
    // 0) recursion stop
    if (depth > maxDepth)
        return bgColor;

    // 1) intersect with scene
    Intersection its;
    if (!Utils::getClosestIntersection(r, objList, its))
        return bgColor;

    // 2) local shading frame
    const Vector3D x  = its.itsPoint;
    const Vector3D n  = its.normal.normalized();
    const Vector3D wo = (-r.d).normalized();

    const Material& mat = its.shape->getMaterial();

    // 3) ambient + emissive
    Vector3D Lo(0.0f);
    if (mat.hasDiffuseOrGlossy())
        Lo += mat.getDiffuseReflectance() * ambientTerm;
    if (mat.isEmissive())
        Lo += mat.getEmissiveRadiance();

    // 4) direct illumination from (sampled) point/area lights
    for (const LightSource* L : lsList)
    {
        // For point lights this will be constant; for area lights you could stratify/sample multiple times.
        const Vector3D lp = L->sampleLightPosition();
        Vector3D Li       = L->getIntensity();

        Vector3D wi = lp - x;
        const double dist2 = wi.lengthSq();
        if (dist2 <= 0.0) continue;
        const double dist = std::sqrt(dist2);
        wi /= dist;

        // Optional inverse-square falloff (keep if getIntensity() is radiant intensity)
        Li = Li / dist2;

        // Shadow ray: rely on maxT to clip beyond the light (no occ.t available)
        Ray shadowRay(x + n * (float)Epsilon, wi);
        shadowRay.minT = Epsilon;
        shadowRay.maxT = dist - Epsilon;

        Intersection occ;
        const bool blocked = Utils::getClosestIntersection(shadowRay, objList, occ);
        if (blocked)
            continue; // Vs(x)=0

        // BRDF evaluation (Phong/diffuse handled inside the material)
        const Vector3D fr = mat.getReflectance(n, wo, wi);

        // Add contribution: Li * fr * max(0, n·wi)
        const double nDotWi = std::max(0.0, dot(n, wi));
        Lo += Li * fr * nDotWi;
    }

    // 5) (optional) recursive mirror/refraction can be added later using hasSpecular()/hasTransmission()

    return Lo;
}
