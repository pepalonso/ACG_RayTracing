#include "phong.h"

#include <iostream>

Phong::Phong()
{ }

Phong::Phong(Vector3D Kd_, Vector3D Ks_, float alpha_):
rho_d(Kd_), Ks(Ks_), alpha(alpha_){}


Vector3D Phong::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    //FILL(...)
        // 1) Normalize inputs so dot products are true cosines
        Vector3D N  = n;  N.normalized();
        Vector3D Wi = wi; Wi.normalized();   // incoming light dir (point -> light)
        Vector3D Wo = wo; Wo.normalized();   // outgoing/view dir (point -> camera)
        // 2) Lambertian diffuse: (ρ_d / π) * max(0, n · wi)
        double ndotwi = std::max(0.0, dot(N,Wi));//max(0,.) kills light coming from behind
        Vector3D diffuse = rho_d * (ndotwi / M_PI);
        // 3) Ideal mirror reflection direction: ω_r = 2 (n · wi) n − wi
        Vector3D Wr = (2.0 * dot(N,Wi)) * N - Wi;
        Wr.normalized();
        // 4) Phong specular lobe: k_s * (max(0, ω_o · ω_r))^α
        double rdotwo = std::max(0.0, dot(Wr,Wo));
        Vector3D specular = Ks * std::pow(rdotwo, alpha);
        // 5) Total BRDF value
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

