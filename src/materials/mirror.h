#ifndef MIRRORMATERIAL
#define MIRRORMATERIAL

#include "material.h"

class Mirror : public Material
{
public:
    Mirror();
    Mirror(Vector3D reflectance_);

    Vector3D getReflectance(const Vector3D& n, const Vector3D& wo,
        const Vector3D& wi) const;

    bool hasSpecular() const { return true; }
    bool hasTransmission() const { return false; }
    bool hasDiffuseOrGlossy() const { return false; }
    bool isEmissive() const { return false; }

    double getIndexOfRefraction() const;
    Vector3D getEmissiveRadiance() const;
    Vector3D getDiffuseReflectance() const;
    
    Vector3D getReflectanceCoefficient() const { return reflectance; }

private:
    Vector3D reflectance;
};

#endif // MIRRORMATERIAL

