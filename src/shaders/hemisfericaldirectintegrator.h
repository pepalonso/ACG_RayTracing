#ifndef HEMISPHERICALDIRECTINTEGRATOR_H
#define HEMISPHERICALDIRECTINTEGRATOR_H

#include "shader.h"

class HemisphericalDirectIntegrator : public Shader
{
public:
    HemisphericalDirectIntegrator(Vector3D bgColor_, int numSamples_);
    
    virtual Vector3D computeColor(const Ray &r,
                                 const std::vector<Shape*> &objList,
                                 const std::vector<LightSource*> &lsList) const;

private:
    int numSamples;
};

#endif // HEMISPHERICALDIRECTINTEGRATOR_H

