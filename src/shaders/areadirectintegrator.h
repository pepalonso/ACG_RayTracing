#ifndef AREADIRECTINTEGRATOR_H
#define AREADIRECTINTEGRATOR_H

#include "shader.h"
#include "../core/vector3d.h"
#include "../core/ray.h"
#include <vector>

// Forward declarations
class Shape;
class LightSource;
class Material;

class AreaDirectIntegrator : public Shader
{
public:
    AreaDirectIntegrator(Vector3D bgColor_, int numSamples_);
    
    virtual Vector3D computeColor(const Ray &r,
                                 const std::vector<Shape*> &objList,
                                 const std::vector<LightSource*> &lsList) const override;

private:
    int numSamples;
    
    // Helper methods for area light integration
    Vector3D computeDirectIllumination(const Vector3D& x, const Vector3D& n, 
                                      const Vector3D& wo, const Material& mat,
                                      const std::vector<Shape*>& objList, 
                                      const std::vector<LightSource*>& lsList) const;
    
    Vector3D computeGeometricTerm(const Vector3D& x, const Vector3D& y, 
                                  const Vector3D& nx, const Vector3D& ny) const;
    
    bool computeVisibility(const Vector3D& x, const Vector3D& y, 
                          const std::vector<Shape*>& objList) const;
    
};

#endif // AREADIRECTINTEGRATOR_H
