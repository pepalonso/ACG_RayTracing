// #include "nexteventestimatorintegrationextended.h"
// #include "core/hemisphericalsampler.h"
// #include "core/intersection.h"
// #include "core/utils.h"
// #include "core/vector3d.h"
// #include "core/ray.h"
// #include "shapes/shape.h"
// #include "lightsources/lightsource.h"
// #include <cmath>

// #ifndef M_PI
// #define M_PI 3.14159265358979323846
// #endif

// NextEventEstimatorIntegratorExtended::NextEventEstimatorIntegratorExtended(Vector3D bgColor_, int maxDepth_):
//     Shader(bgColor_), maxDepth(maxDepth_)
// { }

// Vector3D NextEventEstimatorIntegratorExtended::computeColor(
//     const Ray &ray,
//     const std::vector<Shape*> &objList,
//     const std::vector<LightSource*> &lsList) const
// {
//     Intersection its;
//     if (!Utils::getClosestIntersection(ray, objList, its))
//         return bgColor;

//     const Vector3D x  = its.itsPoint;
//     const Vector3D n  = its.normal.normalized();
//     const Vector3D wo = (-ray.d).normalized();
//     const Material& mat = its.shape->getMaterial();

//     // Always pick up emission at the hit
//     Vector3D Lo = mat.isEmissive() ? mat.getEmissiveRadiance() : Vector3D(0.0f);

//     // Reflected radiance (direct + indirect); initial hit didn't arrive from a delta chain
//     Lo += computeReflectedRadiance(x, n, wo, mat, ray.depth, false, objList, lsList);
//     return Lo;
// }

// // split reflected radiance into direct + indirect
// Vector3D NextEventEstimatorIntegratorExtended::computeReflectedRadiance(
//     const Vector3D& x, const Vector3D& n, const Vector3D& wo,
//     const Material& mat, int depth, bool arrivedFromDelta,
//     const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
// {
//     if (depth >= maxDepth) return Vector3D(0.0f);

//     Vector3D Lo(0.0f);

//     // NEE rule: if we reached x via a delta chain, collect emission here
//     if (arrivedFromDelta && mat.isEmissive())
//         Lo += mat.getEmissiveRadiance();

//     const bool isDelta = (mat.hasSpecular() || mat.hasTransmission());

//     // Direct lighting only for non-delta (Phong) materials
//     if (!isDelta)
//         Lo += computeDirectRadiance(x, n, wo, mat, objList, lsList);

//     // Indirect continuation
//     Lo += computeIndirectRadiance(x, n, wo, mat, depth, objList, lsList);

//     return Lo;
// }

// // direct illumination via area light sampling
// static inline double clamp01(double v){ return v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v); }

// Vector3D NextEventEstimatorIntegratorExtended::computeDirectRadiance(
//     const Vector3D& x, const Vector3D& n, const Vector3D& wo,
//     const Material& mat, const std::vector<Shape*>& objList,
//     const std::vector<LightSource*>& lsList) const
// {
//     Vector3D L_dir(0.0f);

//     for (const LightSource* light : lsList)
//     {
//         // 1) Sample 1 point on the area light
//         const Vector3D y  = light->generateRandomPoint();
//         const double   A  = light->getArea();
//         const double   pdf= (A > 0.0 ? 1.0 / A : 0.0);

//         const Vector3D wi = (y - x).normalized();
//         const double   r2 = (y - x).lengthSq();

//         // 2) Visibility test with safe offset and bounds
//         Ray shadowRay(x + n*(float)Epsilon, wi);
//         shadowRay.minT = Epsilon;
//         shadowRay.maxT = std::sqrt(r2) - Epsilon;
//         if (Utils::hasIntersection(shadowRay, objList)) continue; // V=0

//         // 3) Geometric term G(x,y)
//         const Vector3D ny = light->getNormal();    // normal at the light
//         const double nx_wi  = clamp01(dot(n,  wi));
//         const double ny_mwi = clamp01(dot(ny, -wi));
//         const double G = (r2 > 0.0) ? (nx_wi * ny_mwi) / r2 : 0.0;

//         // 4) Contribution
//         const Vector3D Le = light->getIntensity();          // Le(y, -wi)
//         const Vector3D fr = mat.getReflectance(n, wo, wi);  // BRDF at x
//         if (pdf > 0.0)
//             L_dir += (Le * fr) * (float)(G / pdf);
//     }
//     return L_dir;
// }

// // indirect illumination via hemisphere sampling
// Vector3D NextEventEstimatorIntegratorExtended::computeIndirectRadiance(
//     const Vector3D& x, const Vector3D& n, const Vector3D& wo,
//     const Material& mat, int depth,
//     const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const
// {
//     // If delta material: deterministic continuation (reflection / refraction)
//     if (mat.hasSpecular())
//     {
//         // ideal mirror reflection
//         const Vector3D wi = (2.0 * dot(n, wo) * n - wo).normalized();
//         Ray r(x + n*(float)Epsilon, wi, depth + 1);
//         Intersection its;
//         if (Utils::getClosestIntersection(r, objList, its))
//         {
//             const Material& mat2 = its.shape->getMaterial();
//             const Vector3D  x2 = its.itsPoint;
//             const Vector3D  n2 = its.normal.normalized();
//             const Vector3D  wo2= -wi;

//             // propagate with arrivedFromDelta=true so we pick up Le at the next hit
//             return computeReflectedRadiance(x2, n2, wo2, mat2, depth + 1, true, objList, lsList);
//         }
//         return Vector3D(0.0f);
//     }
//     if (mat.hasTransmission())
//     {
//         // perfect refraction (Snell); handle TIR
//         const double nDotWo = dot(n, wo);
//         Vector3D n_face = n;
//         double eta = mat.getIndexOfRefraction(); // µ_t = η1/η2; adapt to your API
//         bool entering = nDotWo > 0.0;
//         if (!entering){ n_face = -n; eta = 1.0 / eta; }

//         const double cosi = std::abs(dot(n_face, wo));
//         const double k = 1.0 - eta*eta*(1.0 - cosi*cosi);
//         if (k < 0.0)
//         {
//             // TIR → reflect instead
//             const Vector3D wiR = (2.0 * dot(n, wo) * n - wo).normalized();
//             Ray rR(x + n*(float)Epsilon, wiR, depth + 1);
//             Intersection itsR;
//             if (Utils::getClosestIntersection(rR, objList, itsR))
//             {
//                 const Material& mR = itsR.shape->getMaterial();
//                 return computeReflectedRadiance(itsR.itsPoint, itsR.normal.normalized(), -wiR, mR,
//                                                 depth + 1, true, objList, lsList);
//             }
//             return Vector3D(0.0f);
//         }
//         const Vector3D wiT = (-eta * wo + n_face * (eta * cosi - std::sqrt(k))).normalized();
//         Ray rT(x - n_face*(float)Epsilon, wiT, depth + 1);
//         Intersection itsT;
//         if (Utils::getClosestIntersection(rT, objList, itsT))
//         {
//             const Material& mT = itsT.shape->getMaterial();
//             return computeReflectedRadiance(itsT.itsPoint, itsT.normal.normalized(), -wiT, mT,
//                                             depth + 1, true, objList, lsList);
//         }
//         return Vector3D(0.0f);
//     }

//     // Otherwise: diffuse/glossy continuation (hemisphere MC)
//     HemisphericalSampler sampler;
//     const Vector3D wi  = sampler.getSample(n).normalized();
//     const double   pdf = 1.0 / (2.0 * M_PI);                     // uniform hemisphere pdf
//     const double   cos = std::max(0.0, dot(n, wi));

//     Ray newRay(x + n*(float)Epsilon, wi, depth + 1);
//     Intersection its;
//     if (!Utils::getClosestIntersection(newRay, objList, its))
//         return Vector3D(0.0f);

//     const Material& mat2 = its.shape->getMaterial();
//     const Vector3D  x2   = its.itsPoint;
//     const Vector3D  n2   = its.normal.normalized();
//     const Vector3D  wo2  = -wi;

//     // continue path (arrivedFromDelta=false because this was a diffuse sample)
//     const Vector3D Lr = computeReflectedRadiance(x2, n2, wo2, mat2, depth + 1, false, objList, lsList);
//     const Vector3D fr = mat.getReflectance(n, wo, wi);  // BRDF at x (no cosine inside)
//     return (pdf > 0.0) ? (Lr * fr * (float)(cos / pdf)) : Vector3D(0.0f);
// }

// // compute geometric term G(x,y)
// Vector3D NextEventEstimatorIntegratorExtended::computeGeometricTerm(const Vector3D& x,
//                                                             const Vector3D& y,
//                                                             const Vector3D& nx, 
//                                                             const Vector3D& ny) const
// {
//     const Vector3D wi = (y - x).normalized();
//     const double r2   = (y - x).lengthSq();
//     if (r2 <= 0.0) return 0.0;
//     const double nx_wi  = std::max(0.0, dot(nx,  wi));
//     const double ny_mwi = std::max(0.0, dot(ny, -wi));
//     return (nx_wi * ny_mwi) / r2;
// }

// // check visibility between x and y
// bool NextEventEstimatorIntegratorExtended::computeVisibility(const Vector3D& x, 
//                                                      const Vector3D& y,
//                                                      const std::vector<Shape*>& objList) const
// {
//     const Vector3D dir = (y - x).normalized();
//     const double   d   = (y - x).length();
//     Ray shadowRay(x + dir*(float)Epsilon, dir);
//     shadowRay.minT = Epsilon;
//     shadowRay.maxT = d - Epsilon;
//     return !Utils::hasIntersection(shadowRay, objList);
// }