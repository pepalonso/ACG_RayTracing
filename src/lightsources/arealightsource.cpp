#include "arealightsource.h"

AreaLightSource::AreaLightSource(Square* areaLightsource_) :
    myAreaLightsource(areaLightsource_)
{ }



Vector3D AreaLightSource::getIntensity() const
{
    return myAreaLightsource->getMaterial().getEmissiveRadiance();
}


Vector3D AreaLightSource::sampleLightPosition()   const
{
    // Generate two random numbers between 0 and 1
    double u = ((double)rand() / RAND_MAX);
    double v = ((double)rand() / RAND_MAX);
    
    // Use barycentric coordinates to get random point on rectangle
    // Point = corner + u * v1 + v * v2
    Vector3D randomPoint = myAreaLightsource->corner + 
                           u * myAreaLightsource->v1 + 
                           v * myAreaLightsource->v2;
    
    return randomPoint;
}

