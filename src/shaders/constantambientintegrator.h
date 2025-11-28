#ifndef CONSTANTAMBIENTINTEGRATOR_H
#define CONSTANTAMBIENTINTEGRATOR_H

#include "shader.h"

class ConstantAmbientIntegrator : public Shader
{
public:
    ConstantAmbientIntegrator(Vector3D bgColor_, float ambientTerm_);
    
    virtual Vector3D computeColor(const Ray &r,
                                 const std::vector<Shape*> &objList,
                                 const std::vector<LightSource*> &lsList) const;

private:
    float ambientTerm;   // Constant ambient factor (e.g., 0.3)
};

#endif // CONSTANTAMBIENTINTEGRATOR_H

