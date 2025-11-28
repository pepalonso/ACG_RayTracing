#include "ambientocclusionintegrator.h"
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"
#include <cmath>

AmbientOcclusionIntegrator::AmbientOcclusionIntegrator(Vector3D bgColor_, int numSamples_, float maxDistance_) :
    Shader(bgColor_), numSamples(numSamples_), maxDistance(maxDistance_)
{ }

Vector3D AmbientOcclusionIntegrator::computeColor(const Ray &ray,
                                          const std::vector<Shape*> &objList,
                                          const std::vector<LightSource*> &lsList) const
{
    // Step 1: Find closest intersection with scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
        return bgColor;  // Ray escaped scene, return background color

    // Step 2: Get material and surface properties
    const Material& mat = its.shape->getMaterial();
    const Vector3D x = its.itsPoint;              // Surface point
    const Vector3D normal = its.normal.normalized(); // Surface normal

    // Step 3: Handle emissive materials (light sources)
    // Emissive materials: Always bright (they're light sources)
    if (mat.isEmissive())
        return Vector3D(1.0, 1.0, 1.0);  // White - no occlusion for lights
    
    // Step 4: Compute Ambient Occlusion for all other materials
    HemisphericalSampler sampler;
    int blockedRays = 0;  // Count how many rays hit something nearby
    
    // Cast numSamples rays in random hemisphere directions
    for (int i = 0; i < numSamples; i++)
    {
        // Get a random direction in the hemisphere around the normal
        Vector3D wi = sampler.getSample(normal);
        
        // Create occlusion test ray from surface point in direction wi
        Ray occlusionRay(x, wi);
        occlusionRay.minT = Epsilon;  // Avoid self-intersection
        occlusionRay.maxT = maxDistance;  // Only check within maxDistance
        
        // Check if ray hits something within maxDistance
        Intersection occlusionHit;
        if (Utils::getClosestIntersection(occlusionRay, objList, occlusionHit))
        {
            // Ray hit something within maxDistance -> it's occluded
            blockedRays++;
        }
        // If no hit within maxDistance, ray "escaped" -> not occluded
    }
    
    // Step 5: Compute AO factor
    // Occlusion factor: ratio of blocked rays
    float occlusionFactor = (float)blockedRays / (float)numSamples;
    
    // AO value: 1.0 = fully exposed (bright), 0.0 = fully occluded (dark)
    float ao = 1.0f - occlusionFactor;
    
    // Get material diffuse color
    Vector3D materialColor = mat.getDiffuseReflectance();
    
    // Return AO modulated by material color
    return materialColor * ao;
}

