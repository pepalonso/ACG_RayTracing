#ifndef AMBIENTOCCLUSIONINTEGRATOR_H
#define AMBIENTOCCLUSIONINTEGRATOR_H

#include "shader.h"

class AmbientOcclusionIntegrator : public Shader
{
public:
    AmbientOcclusionIntegrator(Vector3D bgColor_, int numSamples_, float maxDistance_);
    
    virtual Vector3D computeColor(const Ray &r,
                                 const std::vector<Shape*> &objList,
                                 const std::vector<LightSource*> &lsList) const;

private:
    int numSamples;      // Number of rays to cast for AO computation
    float maxDistance;   // Maximum distance for occlusion (beyond this = not occluded)
};

#endif // AMBIENTOCCLUSIONINTEGRATOR_H

