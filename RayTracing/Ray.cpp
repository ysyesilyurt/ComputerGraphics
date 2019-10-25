#include "Ray.h"
#include "helpers.h"

Ray::Ray()
{
}

Ray::Ray(const Vector3f& origin, const Vector3f& direction)
    : origin(origin), direction(direction)
{
}

/* Takes a parameter t and returns the point accoring to t. t is the parametric variable in the ray equation o+t*d.*/
Vector3f Ray::getPoint(float t) const 
{
    return this->origin + (this->direction * t);
}

/* Takes a point p and returns the parameter t according to p such that p = o+t*d. */
float Ray::gett(const Vector3f & p) const
{
    Vector3f diff = p - this->origin;
    
    return (diff.x + diff.y + diff.z)  / (this->direction.x + this->direction.y + this->direction.z);
}

