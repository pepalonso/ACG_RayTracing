#include "transmissive.h"

Transmissive::Transmissive()
{ }

Transmissive::Transmissive(double mu_t_):
    mu_t(mu_t_)
{ }


Vector3D Transmissive::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    // Perfect transmission - this function is not used
    // Transmissive materials handle refraction recursively in the integrator
    return Vector3D(0.0);
}


double Transmissive::getIndexOfRefraction() const
{
    return mu_t;
}


Vector3D Transmissive::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Transmissive::getDiffuseReflectance() const
{
    return Vector3D(0.0);
}

