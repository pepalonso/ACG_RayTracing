#include "depthshader.h"

#include "core/utils.h"

DepthShader::DepthShader() :
    color(Vector3D(1, 0, 0))
{ }

DepthShader::DepthShader(Vector3D hitColor_, double maxDist_, Vector3D bgColor_) :
    Shader(bgColor_), maxDist(maxDist_), color(hitColor_)
{ }

Vector3D DepthShader::computeColor(const Ray &r, const std::vector<Shape*> &objList, const std::vector<LightSource*> &lsList) const
{
    //if..
    Intersection its;
    if(Utils::getClosestIntersection(r, objList, its)){
        Vector3D origin = r.o;
        Vector3D destination = its.itsPoint;
        double distance = (destination - origin).length();
        double dist_n = distance / maxDist;
        dist_n = std::clamp(dist_n, 0.0, 1.0);
        return Vector3D(0, 1-dist_n, 0);
    }
    //else...

    return bgColor;
}
