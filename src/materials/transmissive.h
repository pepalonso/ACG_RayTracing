#ifndef TRANSMISSIVEMATERIAL
#define TRANSMISSIVEMATERIAL

#include "material.h"

class Transmissive : public Material
{
public:
    Transmissive();
    Transmissive(double mu_t_);

    Vector3D getReflectance(const Vector3D& n, const Vector3D& wo,
        const Vector3D& wi) const;

    bool hasSpecular() const { return false; }
    bool hasTransmission() const { return true; }
    bool hasDiffuseOrGlossy() const { return false; }
    bool isEmissive() const { return false; }

    double getIndexOfRefraction() const;
    Vector3D getEmissiveRadiance() const;
    Vector3D getDiffuseReflectance() const;

private:
    double mu_t;  // Refractive index ratio (η₁/η₂)
};

#endif // TRANSMISSIVEMATERIAL

