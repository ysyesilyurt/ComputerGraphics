#include "Shape.h"
#include "Scene.h"
#include "helpers.h"
#include <cstdio>
#include <limits>


#define verySmall 1e-6
#define nullIntersect {INF,{},-1}

Shape::Shape(void)
{
}

Shape::Shape(int id, int matIndex)
    : id(id), matIndex(matIndex)
{
}

Sphere::Sphere(void)
{}

/* Constructor for sphere. You will implement this. */
Sphere::Sphere(int id, int matIndex, int cIndex, float R)
    : Shape(id, matIndex)
{
    this->center = pScene->vertices[cIndex-1];
    this->radiusSquare = R*R;
}

/* Sphere-ray intersection routine. You will implement this. 
Note that IntersectionData structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etp3.
You should to declare the variables in IntersectionData structure you think you will need. It is in defs.h file. */
IntersectionData Sphere::intersect(const Ray & ray) const
{
    // float a = 1;  d^2
    float b = dotProduct(ray.direction, ray.origin - this->center); // d.(o-c)
    float c = dotProduct(ray.origin - this->center, ray.origin - this->center) - this->radiusSquare * this->radiusSquare; // (o-c)^2 - R^2

    float discriminant = b*b - c;

    if(discriminant < verySmall)
        return nullIntersect;

    float t = -b - sqrt(discriminant);

    if(t < verySmall)
        return nullIntersect;

    return {t, ((ray.origin +  ray.direction * t) - this->center).normalize(), matIndex};
}

Triangle::Triangle(void)
{}

/* Constructor for triangle. You will implement this. */
Triangle::Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index)
    : Shape(id, matIndex)
{
    this->p1 = pScene->vertices[p1Index-1];
    this->p2 = pScene->vertices[p2Index-1];
    this->p3 = pScene->vertices[p3Index-1];
}

/* Triangle-ray intersection routine. You will implement this. 
Note that IntersectionData structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etp3.
You should to declare the variables in IntersectionData structure you think you will need. It is in defs.h file. */
IntersectionData Triangle::intersect(const Ray & ray) const
{
    float det = determinant(
            p1.x - p2.x, p1.x - p3.x, ray.direction.x,
            p1.y - p2.y, p1.y - p3.y, ray.direction.y,
            p1.z - p2.z, p1.z - p3.z, ray.direction.z);

    if(det < verySmall && det > -verySmall)
        return nullIntersect;

    float beta = determinant(
            p1.x - ray.origin.x, p1.x - p3.x, ray.direction.x,
            p1.y - ray.origin.y, p1.y - p3.y, ray.direction.y,
            p1.z - ray.origin.z, p1.z - p3.z, ray.direction.z)
                 / det;
    float gamma = determinant(
            p1.x - p2.x, p1.x - ray.origin.x, ray.direction.x,
            p1.y - p2.y, p1.y - ray.origin.y, ray.direction.y,
            p1.z - p2.z, p1.z - ray.origin.z, ray.direction.z)
                 / det;
    float t = determinant(
            p1.x - p2.x, p1.x - p3.x, p1.x - ray.origin.x,
            p1.y - p2.y, p1.y - p3.y, p1.y - ray.origin.y,
            p1.z - p2.z, p1.z - p3.z, p1.z - ray.origin.z)
              / det;

    if(     t < verySmall ||
            (beta + gamma) - 1 > verySmall ||
            beta < verySmall ||
            gamma < verySmall
        ) return nullIntersect;

    return {t, crossProduct(p2-p1, p3-p1).normalize(), matIndex};

}

Mesh::Mesh()
{}

/* Constructor for mesh. You will implement this. */
Mesh::Mesh(int id, int matIndex, const vector<Triangle>& faces)
    : Shape(id, matIndex)
{
    int size = faces.size();
    for(int i=0; i<size; i++) this->triangles[i] = faces[i];
    // TODO: this guy could be a lot faster but less safe           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    //  just make triangles a pointer and assign it to address of faces
}

/* Mesh-ray intersection routine. You will implement this. 
Note that IntersectionData structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etp3.
You should to declare the variables in IntersectionData structure you think you will need. It is in defs.h file. */
IntersectionData Mesh::intersect(const Ray & ray) const
{
    int size = this->triangles.size();
    IntersectionData tempMin = nullIntersect;

    for(int i=0; i < size; i++)
    {
        IntersectionData inters = this->triangles[i].intersect(ray);
        if(inters.t < tempMin.t)
        {
            tempMin = inters;
        }
    }
    return tempMin;

}
