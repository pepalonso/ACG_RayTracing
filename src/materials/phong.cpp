#include "phong.h"

#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    //   - n is the surface normal
    //   - ω_i is the incident light direction (towards the light)
    //   - ω_o is the outgoing view direction (towards the camera)
    //   - ω_r is the ideal reflection direction
    
    // Lambertian diffuse term: ρ_d / π
    Vector3D diffuse = rho_d / M_PI;
    
    // Ideal mirror reflection direction: ω_r = 2(n · ω_i)n - ω_i  [Equation 5]
    Vector3D wr = (2.0 * dot(n, wi)) * n - wi;
    wr = wr.normalized();
    
    // Phong specular lobe: k_s * (ω_o · ω_r)^α
    // Note: max(0, .) ensures no contribution when viewing from behind the reflection
    double wo_dot_wr = std::max(0.0, dot(wo, wr));
    Vector3D specular = Ks * std::pow(wo_dot_wr, alpha);
    
    // Total BRDF: diffuse + specular components
    return diffuse + specular;
};


double Phong::getIndexOfRefraction() const
{
    std::cout << "Warning! Calling \"Material::getIndexOfRefraction()\" for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Phong::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Phong::getDiffuseReflectance() const
{
    return rho_d;
}

