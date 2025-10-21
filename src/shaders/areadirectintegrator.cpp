#include "areadirectintegrator.h"
#include "../core/utils.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AreaDirectIntegrator::AreaDirectIntegrator(Vector3D bgColor_, int numSamples_) :
    Shader(bgColor_), numSamples(numSamples_)
{ }

Vector3D AreaDirectIntegrator::computeColor(const Ray &ray,
                                           const std::vector<Shape*> &objList,
                                           const std::vector<LightSource*> &lsList) const
{
    // Step 1: Find closest intersection with scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
        return bgColor;  // Ray escaped scene, return background color

    // Step 2: Setup local shading frame at intersection point
    const Vector3D x = its.itsPoint;           // Surface point x
    const Vector3D normal = its.normal.normalized(); // Surface normal n_x
    const Vector3D wo = (-ray.d).normalized();   // Outgoing direction ω_o (towards camera)
    const Material& surfaceMaterial = its.shape->getMaterial();

    // Step 3: Initialize outgoing radiance
    Vector3D Lo(0.0f);  // L_o(x, ω_o) - outgoing radiance
    
    // Add emissive contribution if material is emissive
    if (surfaceMaterial.isEmissive())
        Lo += surfaceMaterial.getEmissiveRadiance();

    // Step 4: Direct Illumination from Area Light Sources
    if (surfaceMaterial.hasDiffuseOrGlossy())
    {
        for (const LightSource* L : lsList)  // Loop over all light sources
        {
            for (int i = 0; i < numSamples; i++)
            {
                // Sample random point on light source
                const Vector3D y = L->sampleLightPosition();
                
                // Compute direction from surface point to light point
                Vector3D wi = y - x;
                const double dist2 = wi.lengthSq();  // Squared distance
                const double dist = std::sqrt(dist2); // Actual distance
                wi /= dist;  // Normalize direction

                // Check visibility (shadow ray)
                Ray shadowRay(x, wi);
                shadowRay.minT = Epsilon;
                shadowRay.maxT = dist - Epsilon;

                Intersection occ;
                const bool blocked = Utils::getClosestIntersection(shadowRay, objList, occ);
                if (blocked)
                    continue;

                // Light is visible
                
                // Get light intensity
                Vector3D Le = L->getIntensity();
                
                // Evaluate BRDF: f_r(n_x, ω_i, ω_o)
                const Vector3D fr = surfaceMaterial.getReflectance(normal, wo, wi);
                
                // Compute geometric term G(x,y) = (n_x · ω_i)(n_y · (-ω_i)) / ||y-x||²
                Vector3D ny = L->getNormal();  // Light source normal
                const double G = (dot(normal, wi) * dot(ny, -wi)) / dist2;
                
                // PDF for uniform area sampling: p(y) = 1/A
                const double pdf = 1.0 / L->getArea();
                
                // Monte Carlo estimator: L_e * f_r * G / p(y)
                Lo += Le * fr * G / pdf;
            }
        }
    }

    return Lo;
}
