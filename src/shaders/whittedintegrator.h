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
    // Matches your .cpp signature
    WhittedIntegrator(Vector3D& bgColor, int maxDepth_, float ambientTerm_);

    // Framework entry point: the raytracer calls this
    Vector3D computeColor(const Ray &r,
                          const std::vector<Shape*> &objList,
                          const std::vector<LightSource*> &lsList) const override;

    // Your recursive core (called by computeColor)
    Vector3D computeColorRecursive(const Ray &r,
                                   const std::vector<Shape*> &objList,
                                   const std::vector<LightSource*> &lsList,
                                   int depth) const;

private:
    int   maxDepth;     // recursion limit (inclusive check in your code uses depth > maxDepth)
    float ambientTerm;  // k_a scalar for ambient contribution
};

#endif // WHITTEDINTEGRATOR_H
