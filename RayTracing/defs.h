#ifndef _DEFS_H_
#define _DEFS_H_

#include <cmath>

class Scene;

/* 3 dimensional vector holding floating point numbers.
Used for both coordinates and color. 
Use x, y, z for coordinate computations, and use r, g, b for color computations. 
Note that you do not have to use this one if you use any vector computation library like Eigen. */
typedef struct Vector3f
{
	union 
	{
		float x;
		float r;
	};
	union
	{
		float y;
		float g;
	};
	union
	{
		float z;
		float b;
	};

	inline Vector3f operator+(Vector3f a) const {
        return {x+a.x, y+a.y, z+a.z};
    }

	inline Vector3f operator-(Vector3f a) const {
		return {x-a.x, y-a.y, z-a.z};
    }

	inline Vector3f operator*(float a) const {
		return {x*a, y*a, z*a};
    }


	inline Vector3f operator/(float a) const {
		return {x/a, y/a, z/a};
    }

	inline Vector3f& operator=(Vector3f a) {
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
    }

    inline Vector3f& operator+=(Vector3f a) {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }

    inline bool operator==(Vector3f a) const {
       return (a.x == x && a.y == y && a.z == z);
    }

    inline bool operator!=(Vector3f a) const {
       return (a.x != x || a.y != y || a.z != z);
    }

    inline float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    inline Vector3f& normalize() {
	    // This function normalizes this and returns it
	    // So can be used with assignments such as a = abc.normalize();
        float len = length();
        if (len == 0.0f) { // len is zero then return zero vector
            x = 0;
            y = 0;
            z = 0;
            return *this;
        }
        x /= len;
        y /= len;
        z /= len;
        return *this;
    }
} Vector3f;

/* Structure to hold return value from ray intersection routine. 
This should hold information related to the intersection point, 
for example, coordinate of the intersection point, surface normal at the intersection point etc. 
Think about the variables you will need for this purpose and declare them here inside of this structure. */
typedef struct IntersectionData
{

	float t;
	Vector3f normal;
    int materialId;

} IntersectionData;


//
// The global variable through which you can access the scene data
//
extern Scene* pScene;

#endif
