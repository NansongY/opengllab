#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Homogeneous4.h"
#include "Ray.h"
#include "Material.h"
#include "RGBAImage.h"

class Triangle
{


    public:
        int triangle_id;
        float intersect(Ray r) const;
        Homogeneous4 verts[3];
        Homogeneous4 normals[3];
        Homogeneous4 colors[3];
        Homogeneous4 phongShade(Cartesian3 hitPoint, 
                                Cartesian3 normal,
                                Cartesian3 lightPos, 
                                Cartesian3 lightColor) const;
        Cartesian3 uvs[3];
        Cartesian3 baricentric(Cartesian3 o);

        Material *shared_material;

        Triangle();
        void validate(int id);
        bool isValid();



};

#endif // TRIANGLE_H
