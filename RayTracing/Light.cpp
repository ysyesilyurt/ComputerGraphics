#include "Light.h"
#include "helpers.h"

/* Constructor. Implemented for you. */
PointLight::PointLight(const Vector3f & position, const Vector3f & intensity)
    : position(position), intensity(intensity)
{
}

// Compute the contribution of light at point p using the
// inverse square law formula
Vector3f PointLight::computeLightContribution(const Vector3f& p) {

	/*
	 * Irradiance on a point due to a light source is computed here
	 * using inverse square law formula
	 *
	 * E(d) = I / d^2
	 */

	Vector3f lightDirection = this->position - p;
    float lightDistance = vectorLength(lightDirection);
    Vector3f irradianceContribution = this->intensity / (lightDistance * lightDistance);
    return irradianceContribution;
}
