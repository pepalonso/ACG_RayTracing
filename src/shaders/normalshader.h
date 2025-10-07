#ifndef NORMALSHADER_H
#define NORMALSHADER_H

#include "shader.h"


class NormalShader : public Shader
{
public:
    NormalShader(Vector3D bgColor_)
        : Shader(bgColor_) {} //constructor of n.s shader to initialize bgcolor calling shader class
    Vector3D computeColor(const Ray &r,
                             const std::vector<Shape*> &objList,
                             const std::vector<LightSource*> &lsList) const; //same function as before

};

#endif // !NORMALSHADER_H
