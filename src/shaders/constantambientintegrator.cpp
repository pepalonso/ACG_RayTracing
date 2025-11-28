#include "constantambientintegrator.h"
#include "../core/utils.h"

ConstantAmbientIntegrator::ConstantAmbientIntegrator(Vector3D bgColor_, float ambientTerm_) :
    Shader(bgColor_), ambientTerm(ambientTerm_)
{ }

Vector3D ConstantAmbientIntegrator::computeColor(const Ray &ray,
                                          const std::vector<Shape*> &objList,
                                          const std::vector<LightSource*> &lsList) const
{
    // Step 1: Find closest intersection with scene geometry
    Intersection its;
    if (!Utils::getClosestIntersection(ray, objList, its))
        return bgColor;  // Ray escaped scene, return background color

    // Step 2: Get material and surface properties
    const Material& mat = its.shape->getMaterial();

    // Step 3: Handle emissive materials (light sources)
    if (mat.isEmissive())
        return Vector3D(1.0, 1.0, 1.0);  // White - lights are always bright
    
    // Step 4: Apply constant ambient term to all other materials
    // No occlusion computation - just uniform ambient lighting everywhere
    Vector3D materialColor = mat.getDiffuseReflectance();
    
    // Return material color × constant ambient factor
    return materialColor * ambientTerm;
}

