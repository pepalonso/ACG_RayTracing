#ifndef NEXTEVENTESTIMATORINTEGRATOREXTENDED_H
#define NEXTEVENTESTIMATORINTEGRATOREXTENDED_H

#include "shader.h"

class NextEventEstimatorIntegratorExtended : public Shader
{
public:
    NextEventEstimatorIntegratorExtended(Vector3D bgColor_, int maxDepth_);

    virtual Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const;

private:
    int maxDepth;
    
    Vector3D computeReflectedRadiance(const Vector3D& x,
                                     const Vector3D& n,
                                     const Vector3D& wo,
                                     const Material& mat,
                                     int depth,
                                     const std::vector<Shape*>& objList,
                                     const std::vector<LightSource*>& lsList) const;
    
    Vector3D computeDirectRadiance(const Vector3D& x,
                                  const Vector3D& n,
                                  const Vector3D& wo,
                                  const Material& mat,
                                  const std::vector<Shape*>& objList,
                                  const std::vector<LightSource*>& lsList) const;
    
    Vector3D computeIndirectRadiance(const Vector3D& x,
                                    const Vector3D& n,
                                    const Vector3D& wo,
                                    const Material& mat,
                                    int depth,
                                    const std::vector<Shape*>& objList,
                                    const std::vector<LightSource*>& lsList) const;
    
    Vector3D computeGeometricTerm(const Vector3D& x, const Vector3D& y,
                                 const Vector3D& nx, const Vector3D& ny) const;
    
    bool computeVisibility(const Vector3D& x, const Vector3D& y,
                          const std::vector<Shape*>& objList) const;
};

#endif // NEXTEVENTESTIMATORINTEGRATOREXTENDED_H
