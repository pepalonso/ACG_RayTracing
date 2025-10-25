#ifndef PUREPATHTRACINGINTEGRATOR_H
#define PUREPATHTRACINGINTEGRATOR_H

#include "shader.h"

class PurePathTracingIntegrator : public Shader
{
public:
    PurePathTracingIntegrator(Vector3D bgColor_, int maxDepth_);
    
    virtual Vector3D computeColor(const Ray &r,
                                 const std::vector<Shape*> &objList,
                                 const std::vector<LightSource*> &lsList) const;

private:
    int maxDepth;
};

#endif // PUREPATHTRACINGINTEGRATOR_H

