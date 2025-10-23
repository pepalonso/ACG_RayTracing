#include "hemisfericaldirectintegrator.h"
#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

HemisphericalDirectIntegrator::HemisphericalDirectIntegrator(Vector3D bgColor_, int numSamples_) :
    Shader(bgColor_), numSamples(numSamples_)
{ }

Vector3D HemisphericalDirectIntegrator::computeColor(const Ray &ray,
                                           const std::vector<Shape*> &objList,
                                           const std::vector<LightSource*> &lsList) const
{
    // Find closest intersection with scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
        return bgColor;  // Ray escaped scene, return background color

    // Setup local shading frame at intersection point
    const Vector3D x = its.itsPoint;           // Surface point x
    const Vector3D normal = its.normal.normalized(); // Surface normal n_x
    const Vector3D wo = (-ray.d).normalized();   // Outgoing direction ω_o (towards camera)
    const Material& surfaceMaterial = its.shape->getMaterial();

    // Step 3: Initialize outgoing radiance
    Vector3D Lo(0.0f);  // L_o(x, ω_o) - outgoing radiance
    
    // Only compute direct illumination for diffuse/glossy materials
    if (surfaceMaterial.hasDiffuseOrGlossy())
    {
        
        HemisphericalSampler sampler;
        double pwj = 1.0 / (2.0 * M_PI);
        
        for (int i = 0; i < numSamples; i++)
        {
            // get a random direction from the hemisphere sampler
            Vector3D wj = sampler.getSample(normal);
            
            // shadow ray from x in direction wj to find what's there
            Ray shadowRay(x, wj);
            Intersection lightIntersection;
            
            // check if the ray hits anything
            if (Utils::getClosestIntersection(shadowRay, objList, lightIntersection))
            {
                Vector3D y = lightIntersection.itsPoint;  // Point y that we hit
                
                // get the material at point y
                const Material& yMaterial = lightIntersection.shape->getMaterial();
                
                // check if its a light source
                if (yMaterial.isEmissive())
                {
                    // light emitted by y towards wj
                    Vector3D Ldir = yMaterial.getEmissiveRadiance();
                    
                    // get the phong BRDF at point x for wj
                    Vector3D fr = surfaceMaterial.getReflectance(normal, wo, wj);
                    
                    // compute the (ωj · n)
                    double wj_dot_n = dot(wj, normal);
                    
                    
                    // add this sample to the Monte Carlo estimate
                    Lo += (Ldir * fr * wj_dot_n) / pwj;
                }
            }
        }
        
        // average over all samples
        Lo = Lo / numSamples;
    }
    
    // Add emissive radiance if this surface itself is a light source
    if (surfaceMaterial.isEmissive())
    {
        Lo += surfaceMaterial.getEmissiveRadiance();
    }

    // Handle Perfect Specular Reflection (Mirror materials)
    if (surfaceMaterial.hasSpecular())
    {
        // Compute ideal reflection direction: ω_r = 2(n · ω_o)n - ω_o
        Vector3D wr = 2.0 * dot(normal, wo) * normal - wo;
        wr = wr.normalized();

        // Create reflected ray from surface point in direction ω_r
        Ray reflectedRay(x + normal * (float)Epsilon, wr);
        
        // Recursively trace reflected ray
        Vector3D reflectedRadiance = computeColor(reflectedRay, objList, lsList);
        
        // Add reflected contribution weighted by mirror's reflectance
        Lo += reflectedRadiance * surfaceMaterial.getDiffuseReflectance();
    }

    // Handle Perfect Specular Transmission (Refractive materials)
    if (surfaceMaterial.hasTransmission())
    {
        // Determine if ray is entering or exiting the material
        double n_dot_wo = dot(normal, wo);
        bool entering = n_dot_wo > 0;
        
        // Get refractive index ratio μ_t = η₁/η₂
        double mu_t = surfaceMaterial.getIndexOfRefraction();
        
        // Flip normal and mu if exiting (going from denser to less dense medium)
        Vector3D n_refr = entering ? normal : -normal;
        double mu = entering ? mu_t : 1.0 / mu_t;
        double cos_theta = std::abs(n_dot_wo);
        
        // Check for total internal reflection using radicand from Equation 8
        double radicand = 1.0 - mu * mu * (1.0 - cos_theta * cos_theta);
        
        if (radicand >= 0.0)  // No total internal reflection
        {
            // Compute transmission direction: ω_t = -μ·ω_o + n(μ(n·ω_o) - √radicand) [Eq. 8]
            double sqrt_term = std::sqrt(radicand);
            Vector3D wt = -mu * wo + n_refr * (mu * cos_theta - sqrt_term);
            wt = wt.normalized();
            
            // Create transmitted ray (offset in opposite direction of normal)
            Ray transmittedRay(x - n_refr * (float)Epsilon, wt);
            
            // Recursively trace transmitted ray
            Vector3D transmittedRadiance = computeColor(transmittedRay, objList, lsList);
            
            // Add transmitted contribution (no color filtering for pure glass)
            Lo += transmittedRadiance;
        }
        // If radicand < 0: total internal reflection occurs, no transmission
    }

    return Lo;
}
