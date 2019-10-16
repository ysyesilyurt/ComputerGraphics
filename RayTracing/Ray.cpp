#include "Ray.h"

Ray::Ray()
{
}

Ray::Ray(const Vector3f& origin, const Vector3f& direction)
    : origin(origin), direction(direction)
{
}


// Helper function for addition
Vector3f Ray::vectorSum(Vector3f first, Vector3f second, int type) const
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

Vector3f Ray::vectorSubtract(Vector3f first, Vector3f second, int type) const
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

Vector3f Ray::scalarMultipleVector(float scale, Vector3f vector, int type) const
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



/* Takes a parameter t and returns the point accoring to t. t is the parametric variable in the ray equation o+t*d.*/
Vector3f Ray::getPoint(float t) const 
{
	/***********************************************
     *                                             *
	 * TODO: Implement this function               *
     *                                             *
     ***********************************************
	 */
    return this->vectorSum( this->origin, this->scalarMultipleVector(t, this->direction));
}

/* Takes a point p and returns the parameter t according to p such that p = o+t*d. */
float Ray::gett(const Vector3f & p) const
{
	/***********************************************
     *                                             *
	 * TODO: Implement this function               *
     *                                             *
     ***********************************************
	 */
    Vector3f diff = this->vectorSubtract(p,this->origin);
    return (diff.x + diff.y + diff.z)  / (this->direction.x + this->direction.y + this->direction.z);
}

