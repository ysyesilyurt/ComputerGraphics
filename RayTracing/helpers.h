#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <cmath>
#include "defs.h"


inline Vector3f vectorSum(const Vector3f & first, const Vector3f & second, int type = 0)
{
	Vector3f res;

	if(type == 1)
	{
		res.r = first.r + second.r;
		res.g = first.g + second.g;
		res.b = first.b + second.b;
	}
	else 
	{
		res.x = first.x + second.x;
		res.y = first.y + second.y;
		res.z = first.z + second.z;
	}

	return res;
}


inline Vector3f vectorSubtract(const Vector3f & first, const Vector3f & second, int type = 0)
{
	Vector3f res;

	if(type == 1)
	{
		res.r = first.r - second.r;
		res.g = first.g - second.g;
		res.b = first.b - second.b;
	}
	else 
	{
		res.x = first.x - second.x;
		res.y = first.y - second.y;
		res.z = first.z - second.z;
	}

	return res;
}


inline Vector3f scalarMultipleVector(float scale, const Vector3f & vector, int type = 0)
{
	Vector3f res;

	if(type == 1)
	{
		res.r = scale * vector.r;
		res.g = scale * vector.g;
		res.b = scale * vector.b;
	}
	else 
	{
		res.x = scale * vector.x;
		res.y = scale * vector.y;
		res.z = scale * vector.z;
	}

	return res;
}

inline Vector3f scalarDivisionVector(float scale, const Vector3f & vector, int type = 0)
{
    Vector3f res;

    if(type == 1)
    {
        res.r = vector.r / scale;
        res.g = vector.g / scale;
        res.b = vector.b / scale;
    }
    else
    {
        res.x = vector.x / scale;
        res.y = vector.y / scale;
        res.z = vector.z / scale;
    }

    return res;
}

inline float dotProduct(const Vector3f & first, const Vector3f & second)
{
	return (first.x * second.x + first.y * second.y + first.z * second.z);
}


inline Vector3f crossProduct(const Vector3f & first, const Vector3f & second)
{
	Vector3f res;
    res.y = first.z*second.x - first.x*second.z;
    res.x = first.y*second.z - first.z*second.y;
    res.z = first.x*second.y - first.y*second.x;
	
    return res;
}

inline float length(const Vector3f & vector) {
    return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

inline void normalize(Vector3f & vector) { // TODO & ?
    float len = length(vector);
    if (len == 0.0f)
        vector = {0,0,0}; // len is zero then return zero vector
    vector = scalarDivisionVector(len, vector);
}

/*
Vector3f vectorSubtract(Vector3f first, Vector3f second, int type = 0);
Vector3f scalarMultipleVector(float scale, Vector3f vector, int type = 0);
Vector3f crossProduct(Vector3f first, Vector3f second);
int dotProduct(Vector3f first, Vector3f second);
*/



#endif