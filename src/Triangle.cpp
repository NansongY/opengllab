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
    const float eps = 1e-6f; // small epsilon to avoid numerical issues

    // Step 1: get triangle vertices
    Cartesian3 p = verts[0].Point();
    Cartesian3 q = verts[1].Point();
    Cartesian3 r2 = verts[2].Point();

    // Step 2: construct u = q-p, v = r-p  and plane normal n = u × v
    Cartesian3 u = q - p;
    Cartesian3 v = r2 - p;
    Cartesian3 n = u.cross(v);

    // Step 3: plane intersection t = ((p - s)·n) / (l·n)
    float denom = n.dot(r.direction);
    if (std::fabs(denom) < eps)
        return -1.0f;

    float t = n.dot(p - r.origin) / denom;
    if (t <= eps)
        return -1.0f;

    // Step 4: intersection point o = s + l·t
    Cartesian3 o = r.origin + t * r.direction;

    // Step 5: project to 2D PCS by dropping the dominant axis of n
    float ax = std::fabs(n.x), ay = std::fabs(n.y), az = std::fabs(n.z);

    float ox, oy, px2, py2, qx, qy, rx, ry;
    if (ax >= ay && ax >= az) {
        // drop x, keep (y,z)
        ox = o.y; oy = o.z;
        px2 = p.y; py2 = p.z;
        qx = q.y; qy = q.z;
        rx = r2.y; ry = r2.z;
    } else if (ay >= ax && ay >= az) {
        // drop y, keep (x,z)
        ox = o.x; oy = o.z;
        px2 = p.x; py2 = p.z;
        qx = q.x; qy = q.z;
        rx = r2.x; ry = r2.z;
    } else {
        // drop z, keep (x,y)
        ox = o.x; oy = o.y;
        px2 = p.x; py2 = p.y;
        qx = q.x; qy = q.y;
        rx = r2.x; ry = r2.y;
    }

    // Step 6: 2D point-in-triangle test (edge functions)
    float e0 = (qx - px2) * (oy - py2) - (qy - py2) * (ox - px2);
    float e1 = (rx - qx) * (oy - qy) - (ry - qy) * (ox - qx);
    float e2 = (px2 - rx) * (oy - ry) - (py2 - ry) * (ox - rx);

    bool inside = (e0 >= -eps && e1 >= -eps && e2 >= -eps)
               || (e0 <=  eps && e1 <=  eps && e2 <=  eps);

    if (inside)
        return t;

    return -1.0f;
}

Cartesian3 Triangle::baricentric(Cartesian3 o)
{
    Cartesian3 a = verts[0].Point();
    Cartesian3 b = verts[1].Point();
    Cartesian3 c = verts[2].Point();

    Cartesian3 v0 = b - a;
    Cartesian3 v1 = c - a;
    Cartesian3 v2 = o - a;

    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);

    float denom = d00 * d11 - d01 * d01;

    Cartesian3 bc;
    if (std::fabs(denom) < 1e-8f)
    {
        bc.x = 1.0f;
        bc.y = 0.0f;
        bc.z = 0.0f;
        return bc;
    }

    bc.y = (d11 * d20 - d01 * d21) / denom;
    bc.z = (d00 * d21 - d01 * d20) / denom;
    bc.x = 1.0f - bc.y - bc.z;

    return bc;
}

Homogeneous4 Triangle::phongShade(Cartesian3 hitPoint, Cartesian3 normal,
                                   Cartesian3 lightPos, Cartesian3 lightColor,
                                   bool inShadow) const
{
    Cartesian3 e = (Cartesian3(0,0,0) - hitPoint).unit();
    Cartesian3 l = (lightPos - hitPoint).unit();

    // ambient
    Cartesian3 ambient(shared_material->ambient.x * lightColor.x,
                       shared_material->ambient.y * lightColor.y,
                       shared_material->ambient.z * lightColor.z);

    if (inShadow)
    {
        return Homogeneous4(ambient.x, ambient.y, ambient.z, 1.0f);
    }

    // diffuse
    float cosTheta = std::max(0.0f, normal.dot(l));
    Cartesian3 diffuse((shared_material->diffuse.x * lightColor.x) * cosTheta,
                       (shared_material->diffuse.y * lightColor.y) * cosTheta,
                       (shared_material->diffuse.z * lightColor.z) * cosTheta);

    // specular
    Cartesian3 B = (l + e).unit();
    float cosB = std::max(0.0f, normal.dot(B));
    cosB = std::pow(cosB, shared_material->shininess);
    cosB *= cosTheta * (shared_material->shininess + 2.0f) / (2.0f * 3.141592653589793f);

    Cartesian3 specular((shared_material->specular.x * lightColor.x) * cosB,
                        (shared_material->specular.y * lightColor.y) * cosB,
                        (shared_material->specular.z * lightColor.z) * cosB);
    
                        
    return Homogeneous4(ambient.x + diffuse.x + specular.x,
                        ambient.y + diffuse.y + specular.y,
                        ambient.z + diffuse.z + specular.z,
                        1.0f);
}
