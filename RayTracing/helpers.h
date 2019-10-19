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

inline float determinant(
            float x_00,float x_01,float x_02,
            float x_10,float x_11,float x_12,
            float x_20,float x_21,float x_22)
{
    return
        (x_00 * ((x_11 * x_22) - (x_12 * x_21))) +
        (x_01 * ((x_12 * x_20) - (x_10 * x_22))) +
        (x_02 * ((x_10 * x_21) - (x_20 * x_11)));
}

// TODO: Functions for calculating determinant and Cramer's rule !!
// todo: what if vector product = 0


#endif