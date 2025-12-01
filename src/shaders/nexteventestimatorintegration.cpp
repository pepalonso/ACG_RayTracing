#include "nexteventestimatorintegration.h"
#include "core/hemisphericalsampler.h"
#include "core/intersection.h"
#include "core/utils.h"
#include "core/vector3d.h"
#include "core/ray.h"
#include "shapes/shape.h"
#include "lightsources/lightsource.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

NextEventEstimatorIntegrator::NextEventEstimatorIntegrator(Vector3D bgColor_, int maxDepth_, int aoSamples_, float aoMaxDistance_):
    Shader(bgColor_), maxDepth(maxDepth_), aoSamples(aoSamples_), aoMaxDistance(aoMaxDistance_)
{ }

Vector3D NextEventEstimatorIntegrator::computeColor(const Ray &ray, 
                                                    const std::vector<Shape*> &objList, 
                                                    const std::vector<LightSource*> &lsList) const
{
    // Find the closest intersection with the scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
    {
        return bgColor;
    }
    
    const Vector3D x = its.itsPoint;
    const Vector3D n = its.normal.normalized();
    const Vector3D wo = (-ray.d).normalized();  // Direction towards camera
    const Material& material = its.shape->getMaterial();

    // get emissive radiance
    Vector3D Lo = material.getEmissiveRadiance();

    // add reflected radiance (direct + indirect)
    Lo += computeReflectedRadiance(x, n, wo, material, ray.depth, objList, lsList);

    // Apply ambient occlusion if enabled (only for primary rays and non-emissive surfaces)
    if (aoSamples > 0 && ray.depth == 0 && !material.isEmissive())
    {
        float aoFactor = computeAmbientOcclusion(x, n, objList);
        Lo = Lo * aoFactor;
    }

    // Line 5: Return total radiance
    return Lo;
}

// split reflected radiance into direct + indirect
Vector3D NextEventEstimatorIntegrator::computeReflectedRadiance(const Vector3D& x,
                                                                const Vector3D& n,
                                                                const Vector3D& wo,
                                                                const Material& mat,
                                                                int depth,
                                                                const std::vector<Shape*>& objList,
                                                                const std::vector<LightSource*>& lsList) const
{
    if (depth >= maxDepth)
        return Vector3D(0.0);

    // direct illumination
    Vector3D L_dir = computeDirectRadiance(x, n, wo, mat, objList, lsList);

    // indirect illumination
    Vector3D L_ind = computeIndirectRadiance(x, n, wo, mat, depth, objList, lsList);

    // return sum
    return L_dir + L_ind;
}

// direct illumination via area light sampling
Vector3D NextEventEstimatorIntegrator::computeDirectRadiance(const Vector3D& x,
                                                             const Vector3D& n,
                                                             const Vector3D& wo,
                                                             const Material& mat,
                                                             const std::vector<Shape*>& objList,
                                                             const std::vector<LightSource*>& lsList) const
{
    Vector3D L_dir(0.0);

    // Loop over all lights
    for (const LightSource* light : lsList)
    {
        // sample one random point on the light
        Vector3D y = light->generateRandomPoint();
        double pdf = 1.0 / light->getArea();

        // compute direction from x to y
        Vector3D wi = (y - x).normalized();

        // check visibility and compute contribution
        if (computeVisibility(x, y, objList))
        {
            Vector3D Le = light->getIntensity();
            Vector3D fr = mat.getReflectance(n, wo, wi);
            Vector3D G = computeGeometricTerm(x, y, n, light->getNormal());

            // Add contribution: Le * BRDF * G / pdf
            L_dir += (Le * fr * G) / pdf;
        }
    }

    return L_dir;
}

// indirect illumination via hemisphere sampling
Vector3D NextEventEstimatorIntegrator::computeIndirectRadiance(const Vector3D& x,
                                                               const Vector3D& n,
                                                               const Vector3D& wo,
                                                               const Material& mat,
                                                               int depth,
                                                               const std::vector<Shape*>& objList,
                                                               const std::vector<LightSource*>& lsList) const
{
    // sample random hemisphere direction
    HemisphericalSampler sampler;
    Vector3D wi = sampler.getSample(n);
    double pdf = 1.0 / (2.0 * M_PI);

    // create new ray in sampled direction
    Ray newRay(x, wi, depth + 1);

    // initialize indirect radiance
    Vector3D L_ind(0.0);

    // check depth limit
    if (depth < maxDepth)
    {
        // intersect scene
        Intersection its;
        if (Utils::getClosestIntersection(newRay, objList, its))
        {
            Vector3D y = its.itsPoint;
            Vector3D ny = its.normal.normalized();
            Vector3D wo_next = -wi;
            const Material& yMaterial = its.shape->getMaterial();


            Vector3D Lr = computeReflectedRadiance(y, ny, wo_next, yMaterial, 
                                                   depth + 1, objList, lsList);

            // compute BRDF and cosine term
            Vector3D fr = mat.getReflectance(n, wo, wi);
            double nx_dot_wi = dot(n, wi);

            // accumulate indirect contribution
            L_ind = Lr * fr * nx_dot_wi / pdf;
        }
    }

    return L_ind;
}

// compute geometric term G(x,y)
Vector3D NextEventEstimatorIntegrator::computeGeometricTerm(const Vector3D& x, 
                                                            const Vector3D& y,
                                                            const Vector3D& nx, 
                                                            const Vector3D& ny) const
{
    Vector3D wi = (y - x).normalized();
    double distance2 = (y - x).lengthSq();
    double nx_dot_wi = dot(nx, wi);
    double neg_wi_dot_ny = dot(-wi, ny);
    double G = (nx_dot_wi * neg_wi_dot_ny) / distance2;
    return Vector3D(G, G, G);
}

// check visibility between x and y
bool NextEventEstimatorIntegrator::computeVisibility(const Vector3D& x, 
                                                     const Vector3D& y,
                                                     const std::vector<Shape*>& objList) const
{
    Vector3D direction = (y - x).normalized();
    double distance = (y - x).length();
    Ray shadowRay(x, direction);
    shadowRay.maxT = distance - Epsilon;
    return !Utils::hasIntersection(shadowRay, objList);
}

// compute ambient occlusion factor
float NextEventEstimatorIntegrator::computeAmbientOcclusion(const Vector3D& x,
                                                            const Vector3D& n,
                                                            const std::vector<Shape*>& objList) const
{
    HemisphericalSampler sampler;
    int blockedRays = 0;
    
    // Cast aoSamples rays in random hemisphere directions
    for (int i = 0; i < aoSamples; i++)
    {
        // Get a random direction in the hemisphere around the normal
        Vector3D wi = sampler.getSample(n);
        
        // Create occlusion test ray from surface point in direction wi
        Ray occlusionRay(x, wi);
        occlusionRay.minT = Epsilon;  // Avoid self-intersection
        occlusionRay.maxT = aoMaxDistance;  // Only check within maxDistance
        
        // Check if ray hits something within maxDistance
        Intersection occlusionHit;
        if (Utils::getClosestIntersection(occlusionRay, objList, occlusionHit))
        {
            // Ray hit something within maxDistance -> it's occluded
            blockedRays++;
        }
    }
    
    // Compute AO factor
    float occlusionFactor = (float)blockedRays / (float)aoSamples;
    float aoFactor = 1.0f - occlusionFactor;  // 1.0 = fully lit, 0.0 = fully occluded
    
    return aoFactor;
}