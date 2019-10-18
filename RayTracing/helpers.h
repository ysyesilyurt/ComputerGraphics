#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <cmath>
#include "defs.h"

inline float dotProduct(const Vector3f & first, const Vector3f & second)
{
	return (first.x * second.x + first.y * second.y + first.z * second.z);
}


inline Vector3f crossProduct(const Vector3f & first, const Vector3f & second)
{
    // TODO: calculating determinant ?

	Vector3f res;
    res.y = first.z*second.x - first.x*second.z;
    res.x = first.y*second.z - first.z*second.y;
    res.z = first.x*second.y - first.y*second.x;
	
    return res;
}


// TODO: Functions for calculating determinant and Cramer's rule !!
// todo: what if vector product = 0


#endif