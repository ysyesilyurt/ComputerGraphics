#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <vector>
#include "Ray.h"
#include "defs.h"

using namespace std;

// Base class for any shape object
class Shape
{
public: 
	int id;	        // Id of the shape
	int matIndex;	// Material index of the shape

	virtual IntersectionData intersect(const Ray & ray) const = 0; // Pure virtual method for intersection test. You must implement this for sphere, triangle, and mesh.

    Shape(void);
    Shape(int id, int matIndex); // Constructor

private:
	// Write any other stuff here
};

// Class for sphere
class Sphere: public Shape
{
public:
	Sphere(void);	// Constructor
	Sphere(int id, int matIndex, int cIndex, float R);	// Constructor
	IntersectionData intersect(const Ray & ray) const;	// Will take a ray and return a structure related to the intersection information. You will implement this.

private:
	// Write any other stuff here
	int centerIndex;
	float radiusSquare;
};

// Class for triangle
class Triangle: public Shape
{
public:
	Triangle(void);	// Constructor
	Triangle(int id, int matIndex, int p1Index, int p2Index, int p3Index);	// Constructor
	IntersectionData intersect(const Ray & ray) const; // Will take a ray and return a structure related to the intersection information. You will implement this.

private:
	// Write any other stuff here
	int p1index;
	int p2index;
	int p3index;
};

// Class for mesh
class Mesh: public Shape
{
public:
	Mesh(void);	// Constructor
	Mesh(int id, int matIndex, const vector<Triangle>& faces);	// Constructor
	IntersectionData intersect(const Ray & ray) const; // Will take a ray and return a structure related to the intersection information. You will implement this.

private:
	// Write any other stuff here
	vector<Triangle> triangles;
};

#endif
