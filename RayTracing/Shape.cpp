#include "Shape.h"
#include "Scene.h"
#include <cstdio>

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
    this->center = pScene->vertices[cIndex];
    this->radius = R;
}

/* Sphere-ray intersection routine. You will implement this. 
Note that IntersectionData structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etc.
You should to declare the variables in IntersectionData structure you think you will need. It is in defs.h file. */
IntersectionData Sphere::intersect(const Ray & ray) const
{
	/***********************************************
     *                                             *
	 * TODO: Implement this function               *
     *                                             *
     ***********************************************
	 */
}

Triangle::Triangle(void)
{}

/* Constructor for triangle. You will implement this. */
Triangle::Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index)
    : Shape(id, matIndex)
{
    this->p1 = pScene->vertices[p1Index];
    this->p2 = pScene->vertices[p2Index];
    this->p3 = pScene->vertices[p3Index];
}

/* Triangle-ray intersection routine. You will implement this. 
Note that IntersectionData structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etc.
You should to declare the variables in IntersectionData structure you think you will need. It is in defs.h file. */
IntersectionData Triangle::intersect(const Ray & ray) const
{
	/***********************************************
     *                                             *
	 * TODO: Implement this function               *
     *                                             *
     ***********************************************
	 */
}

Mesh::Mesh()
{}

/* Constructor for mesh. You will implement this. */
Mesh::Mesh(int id, int matIndex, const vector<Triangle>& faces)
    : Shape(id, matIndex)
{
    int size = faces.size();
    for(int i=0; i<size; i++) this->triangles[i] = faces[i];
    // this guy could be a lot faster but less safe           @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    //  just make triangles a pointer and assign it to address of faces
}

/* Mesh-ray intersection routine. You will implement this. 
Note that IntersectionData structure should hold the information related to the intersection point, e.g., coordinate of that point, normal at that point etc.
You should to declare the variables in IntersectionData structure you think you will need. It is in defs.h file. */
IntersectionData Mesh::intersect(const Ray & ray) const
{
	/***********************************************
     *                                             *
	 * TODO: Implement this function               *
     *                                             *
     ***********************************************
	 */
}
