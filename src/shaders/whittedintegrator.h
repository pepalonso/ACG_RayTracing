#ifndef WHITTEDINTEGRATOR_H
#define WHITTEDINTEGRATOR_H

#include "shader.h"
#include "../core/vector3d.h"
#include "../core/ray.h"
#include <vector>

// Forward declarations (keep header lightweight)
class Shape;
class LightSource;

class WhittedIntegrator : public Shader
{
public:
    WhittedIntegrator(Vector3D& bgColor, int maxDepth_, float ambientTerm_);

    Vector3D computeColor(const Ray &r,
                          const std::vector<Shape*> &objList,
                          const std::vector<LightSource*> &lsList) const override;

    Vector3D computeColorRecursive(const Ray &r,
                                   const std::vector<Shape*> &objList,
                                   const std::vector<LightSource*> &lsList,
                                   int depth) const;

private:
    int   maxDepth;     // max recursion depth
    float ambientTerm;  // ambient contribution
};

#endif // WHITTEDINTEGRATOR_H
