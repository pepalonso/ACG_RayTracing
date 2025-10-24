#include "whittedintegrator.h"

#include "../core/utils.h"

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
    
    // Step 0: Check recursion depth limit
    if (depth > maxDepth)
        return bgColor;

    // Step 1: Find closest intersection with scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(r, objList, its))
        return bgColor;  // Ray escaped scene, return background color

    // Step 2: Setup local shading frame at intersection point
    const Vector3D x  = its.itsPoint;           // Surface point x
    const Vector3D n  = its.normal.normalized(); // Surface normal n_x
    const Vector3D wo = (-r.d).normalized();     // Outgoing direction ω_o (towards camera)

    const Material& mat = its.shape->getMaterial();

    // Step 3: Initialize outgoing radiance with ambient and emissive terms
    Vector3D Lo(0.0f);  // L_o(x, ω_o) - outgoing radiance
    
    if (mat.hasDiffuseOrGlossy())
        Lo += mat.getDiffuseReflectance() * ambientTerm;  // Ambient: ρ_d * k_a
    if (mat.isEmissive())
        Lo += mat.getEmissiveRadiance();  // Self-emitted light

    // Step 4: Direct Illumination from Point Lights (only for diffuse/glossy materials)
    if (mat.hasDiffuseOrGlossy())
    {
        for (const LightSource* L : lsList)  // Loop over nL light sources
        {
            // Sample light position (for point lights: single position; area lights: random sample)
            const Vector3D lightPos = L->generateRandomPoint();
            Vector3D Li = L->getIntensity();

            // Compute incident direction ω_i^s: from shading point x to light position
            Vector3D wi = lightPos - x;
            const double dist2 = wi.lengthSq();
            if (dist2 <= 0.0) continue;
            const double dist = std::sqrt(dist2);
            wi /= dist;

            // Apply inverse-square falloff: intensity decreases with distance²
            Li = Li / dist2;

            // V_s(x) = 1 if light s is visible from x, 0 otherwise
            Ray shadowRay(x, wi);  // Offset by epsilon to avoid self-intersection
            shadowRay.minT = Epsilon;
            shadowRay.maxT = dist - Epsilon;  // Stop at light position

            Intersection occ;
            const bool blocked = Utils::getClosestIntersection(shadowRay, objList, occ);
            if (blocked)
                continue;  // V_s(x) = 0: light is occluded, skip this light source

            // V_s(x) = 1: light is visible, compute contribution
            
            // Evaluate BRDF: f_r(n_x, ω_i^s, ω_o) using Phong model
            const Vector3D fr = mat.getReflectance(n, wo, wi);

            // Compute cosine term: (ω_i · n_x)
            // Ensures no contribution from light coming from behind the surface
            const double wi_dot_n = std::max(0.0, dot(wi, n));

            // Add contribution from this light source
            Lo += Li * fr * wi_dot_n;  // V_s(x)=1 implied (not blocked)
        }
    }

    // Step 5: Perfect Specular Reflection (Mirror materials)
    if (mat.hasSpecular())
    {
        // Compute ideal reflection direction: ω_r = 2(n · ω_o)n - ω_o
        Vector3D wr = 2.0 * dot(n, wo) * n - wo;
        wr = wr.normalized();

        // Create reflected ray from surface point in direction ω_r
        Ray reflectedRay(x + n * (float)Epsilon, wr, depth + 1);
        
        // Recursively trace reflected ray
        Vector3D reflectedRadiance = computeColorRecursive(reflectedRay, objList, lsList, depth + 1);
        
        // Add reflected contribution weighted by mirror's reflectance
        Lo += reflectedRadiance * mat.getDiffuseReflectance();
    }

    // Step 6: Perfect Specular Transmission (Refractive materials)
    if (mat.hasTransmission())
    {
        // Determine if ray is entering or exiting the material
        double n_dot_wo = dot(n, wo);
        bool entering = n_dot_wo > 0;
        
        // Get refractive index ratio μ_t = η₁/η₂
        double mu_t = mat.getIndexOfRefraction();
        
        // Flip normal and mu if exiting (going from denser to less dense medium)
        Vector3D n_refr = entering ? n : -n;
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
            Ray transmittedRay(x - n_refr * (float)Epsilon, wt, depth + 1);
            
            // Recursively trace transmitted ray
            Vector3D transmittedRadiance = computeColorRecursive(transmittedRay, objList, lsList, depth + 1);
            
            // Add transmitted contribution (no color filtering for pure glass)
            Lo += transmittedRadiance;
        }
        // If radicand < 0: total internal reflection occurs, no transmission
    }

    return Lo;  // Return total outgoing radiance L_o(x, ω_o)
}
