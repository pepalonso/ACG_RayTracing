#include "purepathtracingintegrator.h"
#include "core/hemisphericalsampler.h"
#include "core/intersection.h"
#include "core/utils.h"
#include "core/vector3d.h"
#include "shapes/shape.h"

PurePathTracingIntegrator::PurePathTracingIntegrator(Vector3D bgColor_, int maxDepth_):
    Shader(bgColor_), maxDepth(maxDepth_)
{ }
Vector3D PurePathTracingIntegrator::computeColor(const Ray &ray, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //Find the closest intersection with the scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
    {
        return bgColor;
    }
    
    //properties of the intersection point
    const Vector3D x = its.itsPoint;
    const Vector3D x_normal = its.normal.normalized();
    const Vector3D wo = (-ray.d).normalized();  // Direction towards camera
    const Material& surfaceMaterial = its.shape->getMaterial();

    //initialize the outgoing radiance
    Vector3D Lo = surfaceMaterial.getEmissiveRadiance();

    // Check depth limit - stop recursion if max depth reached
    if (ray.depth >= maxDepth)
        return Lo;

    // Handle specular materials (Mirror materials)
    if (surfaceMaterial.hasSpecular())
    {
        // Perfect Specular Reflection (Mirror materials)
        Vector3D wr = 2.0 * dot(x_normal, wo) * x_normal - wo;
        wr = wr.normalized();

        // Create reflected ray from surface point in direction ω_r
        Ray reflectedRay(x, wr, ray.depth + 1);
        
        // Recursively trace reflected ray
        Vector3D reflectedRadiance = computeColor(reflectedRay, objList, lsList);
        
        // Add reflected contribution weighted by mirror's reflectance
        Lo += reflectedRadiance * surfaceMaterial.getDiffuseReflectance();
    }
    else if (surfaceMaterial.hasTransmission())
    {
        // Perfect Specular Transmission (Refractive materials)
        // Determine if ray is entering or exiting the material
        double n_dot_wo = dot(x_normal, wo);
        bool entering = n_dot_wo > 0;
        
        // Get refractive index ratio μ_t = η₁/η₂
        double mu_t = surfaceMaterial.getIndexOfRefraction();
        
        // Flip x_normal and mu if exiting (going from denser to less dense medium)
        Vector3D n_refr = entering ? x_normal : -x_normal;
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
            
            // Create transmitted ray
            Ray transmittedRay(x, wt, ray.depth + 1);
            
            // Recursively trace transmitted ray
            Vector3D transmittedRadiance = computeColor(transmittedRay, objList, lsList);
            
            // Add transmitted contribution (no color filtering for pure glass)
            Lo += transmittedRadiance;
        }
        // If radicand < 0: total internal reflection occurs, no transmission
    }
    else  // Diffuse/glossy material
    {
        // Monte Carlo hemisphere sampling for diffuse materials
        HemisphericalSampler sampler;
        Vector3D wi = sampler.getSample(x_normal);
        const double pdf = 1.0 / (2.0 * M_PI);

        // Trace ray in sampled direction
        Ray newRay(x, wi, ray.depth + 1);
        double nx_dot_wi = dot(x_normal, wi);
        
        // Accumulate indirect illumination
        Lo += computeColor(newRay, objList, lsList) * 
              surfaceMaterial.getReflectance(x_normal, wo, wi) * 
              nx_dot_wi / pdf;
    }
    
    return Lo;
}

