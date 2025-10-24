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
    // Find closest intersection with scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
        return bgColor;  // Ray escaped scene, return background color

    // Setup local shading frame at intersection point
    const Vector3D x = its.itsPoint;           // Surface point x
    const Vector3D normal = its.normal.normalized(); // Surface normal n_x
    const Vector3D wo = (-ray.d).normalized();   // Outgoing direction ω_o (towards camera)
    const Material& surfaceMaterial = its.shape->getMaterial();

    // Initialize outgoing radiance
    Vector3D Lo(0.0f);  // L_o(x, ω_o) - outgoing radiance
    
    // Handle different material types
    if (surfaceMaterial.hasDiffuseOrGlossy())
    {
        // Use area light integration for diffuse/glossy materials
        Lo += computeDirectIllumination(x, normal, wo, surfaceMaterial, objList, lsList);
    }
    
    // Add emissive radiance if this surface itself is a light source
    if (surfaceMaterial.isEmissive())
    {
        Lo += surfaceMaterial.getEmissiveRadiance();
    }

    // Handle Specular Reflection (Mirror materials)
    if (surfaceMaterial.hasSpecular())
    {
        // Compute ideal reflection direction: ω_r = 2(n · ω_o)n - ω_o
        Vector3D wr = 2.0 * dot(normal, wo) * normal - wo;
        wr = wr.normalized();

        // Create reflected ray from surface point in direction ω_r
        Ray reflectedRay(x, wr);
        
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
            Ray transmittedRay(x, wt);
            
            // Recursively trace transmitted ray
            Vector3D transmittedRadiance = computeColor(transmittedRay, objList, lsList);
            
            // Add transmitted contribution (no color filtering for pure glass)
            Lo += transmittedRadiance;
        }
        // If radicand < 0: total internal reflection occurs, no transmission
    }

    return Lo;
}

Vector3D AreaDirectIntegrator::computeDirectIllumination(const Vector3D& x, const Vector3D& n, 
                                                        const Vector3D& wo, const Material& mat,
                                                        const std::vector<Shape*>& objList, 
                                                        const std::vector<LightSource*>& lsList) const
{
    Vector3D Lo(0.0f);
        
    for (const LightSource* light : lsList)
    {
        // Only process area lights (skip point lights)
        if (light->getArea() > 0.0)
        {
            Vector3D lightContribution(0.0f);
            
            // Sample the area light source
            for (int i = 0; i < numSamples; i++)
            {
                // Sample random point on light source
                Vector3D y = light->generateRandomPoint();
                
                // Compute direction from x to y
                Vector3D wi = (y - x).normalized();
                
                // Check visibility (shadow test)
                if (computeVisibility(x, y, objList))
                {
                    // Get emitted radiance from light source
                    Vector3D Le = light->getIntensity();
                    
                    // Evaluate BRDF at point x
                    Vector3D fr = mat.getReflectance(n, wo, wi);
                    
                    // Compute geometric term G(x, y)
                    Vector3D G = computeGeometricTerm(x, y, n, light->getNormal());
                    
                    // PDF for uniform area sampling: p(y) = 1/A
                    double pdf = 1.0 / light->getArea();
                    
                    // Add contribution to this sample
                    lightContribution += (Le * fr * G) / pdf;
                }
            }
            
            // Average over samples for this light source
            lightContribution = lightContribution / numSamples;
            Lo += lightContribution;
        }
    }
    
    return Lo;
}

Vector3D AreaDirectIntegrator::computeGeometricTerm(const Vector3D& x, const Vector3D& y, 
                                                   const Vector3D& nx, const Vector3D& ny) const
{
    // G(x, y) = (nx · ωi) (-ωi · ny) / ||y - x||^2
    // Where ωi is the direction from x to y
    
    Vector3D wi = (y - x).normalized();  // Direction from x to y
    double distance2 = (y - x).lengthSq();  // Squared distance
    
    // terms with clamping to avoid negative contributions
    double nx_dot_wi = dot(nx, wi);        // (nx · ωi)
    double neg_wi_dot_ny = dot(-wi, ny);   // (-ωi · ny)
    
    // Geometric term: accounts for surface orientation and distance attenuation
    double G = (nx_dot_wi * neg_wi_dot_ny) / distance2;
    
    return Vector3D(G, G, G);  // Return as Vector3D for consistency
}

bool AreaDirectIntegrator::computeVisibility(const Vector3D& x, const Vector3D& y, 
                                            const std::vector<Shape*>& objList) const
{
    // V(x, y) = 1 if point y is visible from x, 0 otherwise
    // This is a shadow ray test
    
    Vector3D direction = (y - x).normalized();
    double distance = (y - x).length();
    
    // Create shadow ray from x in direction of y
    Ray shadowRay(x, direction);
    shadowRay.maxT = distance - Epsilon;  // Stop at y
        
    // Check if any object blocks the path from x to y
    return !Utils::hasIntersection(shadowRay, objList);
}

