#include "mirror.h"

#include <iostream>

Mirror::Mirror()
{ }

Mirror::Mirror(Vector3D reflectance_):
    reflectance(reflectance_)
{ }


Vector3D Mirror::getReflectance(const Vector3D& n, const Vector3D& wo,
    const Vector3D& wi) const {
    // Mirror materials handle reflection recursively in the integrator
    return Vector3D(0.0);
}


double Mirror::getIndexOfRefraction() const
{
    std::cout << "Calling getIndexOfRefraction() for a non-transmissive material"
              << std::endl;

    return -1;
}


Vector3D Mirror::getEmissiveRadiance() const
{
    return Vector3D(0.0);
}


Vector3D Mirror::getDiffuseReflectance() const
{
    return reflectance;
}

