#include "Triangle.h"
#include <math.h>
#include <algorithm>

Triangle::Triangle()
{
    triangle_id = -1;
    shared_material= nullptr;
}


void Triangle::validate(int id){
    triangle_id = id;
}

bool Triangle::isValid(){
    return triangle_id != -1;
}

float Triangle::intersect(Ray r) const
{
    const float eps = 1e-6f;

    Cartesian3 a = verts[0].Point();
    Cartesian3 b = verts[1].Point();
    Cartesian3 c = verts[2].Point();

    Cartesian3 ab = b - a;
    Cartesian3 ac = c - a;
    Cartesian3 n = ab.cross(ac);

    float denom = n.dot(r.direction);
    if (std::fabs(denom) < eps)
        return -1.0f;

    float t = n.dot(a - r.origin) / denom;
    if (t <= eps)
        return -1.0f;

    Cartesian3 p = r.origin + t * r.direction;

    float h0 = (b - a).cross(p - a).dot(n);
    float h1 = (c - b).cross(p - b).dot(n);
    float h2 = (a - c).cross(p - c).dot(n);

    bool allPos = (h0 >= -eps) && (h1 >= -eps) && (h2 >= -eps);
    bool allNeg = (h0 <= eps) && (h1 <= eps) && (h2 <= eps);

    if (allPos || allNeg)
        return t;

    return -1.0f;
}