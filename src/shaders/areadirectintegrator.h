#ifndef AREADIRECTINTEGRATOR_H
#define AREADIRECTINTEGRATOR_H

#include "shader.h"

class AreaDirectIntegrator : public Shader
{
public:
    AreaDirectIntegrator(Vector3D bgColor_, int numSamples_);
    
    virtual Vector3D computeColor(const Ray &r,
                                 const std::vector<Shape*> &objList,
                                 const std::vector<LightSource*> &lsList) const;

private:
    int numSamples;
};

#endif // AREADIRECTINTEGRATOR_H

