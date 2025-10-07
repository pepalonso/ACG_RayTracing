#include "normalshader.h"

#include "core/utils.h"


Vector3D NormalShader::computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const
{
    Intersection its;
    if(Utils::getClosestIntersection(r, objList, its)) {
        Vector3D n = its.normal; //extract surface normal at hit point
        Vector3D color = (n+Vector3D(1.0,1.0,1.0)) *0.5;//computations of slides
        return color;

    }
    
    return bgColor;
}   
