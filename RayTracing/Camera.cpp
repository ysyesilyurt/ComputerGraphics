#include <cstring>
#include "Camera.h"
#include "helpers.h"

Camera::Camera(int id,                      // Id of the camera
               const char* imageName,       // Name of the output PPM file 
               const Vector3f& pos,         // Camera position
               const Vector3f& gaze,        // Camera gaze direction
               const Vector3f& up,          // Camera up direction
               const ImagePlane& imgPlane)  // Image plane parameters
{
	 this->id = id;
	 strncpy(this->imageName, imageName, 32);
	 this->imgPlane = imgPlane;
	 this->pos = pos;
	 this->gaze = gaze;
	 this->up = up;
	 this->right = crossProduct(gaze, up); // !!
}

/* Takes coordinate of an image pixel as row and col, and
 * returns the ray going through that pixel. 
 */
Ray Camera::getPrimaryRay(int row, int col) const
{
	/* m = e + (-w) * distance */
    /* q = m + l * u + t * v */
    /* s = q + s_u * u - s_v * v */
    /* d = s - e */

    Vector3f origin = this->pos;

    // Vector3f imageCenter = vectorSum(origin, (scalarMultipleVector(this->imgPlane.distance, this->gaze))); // m
     Vector3f imageCenter = origin + (this->gaze * this->imgPlane.distance); // m

//    Vector3f topLeft = vectorSum(imageCenter,
//            vectorSum((scalarMultipleVector(this->imgPlane.left, this->right)),
//                    (scalarMultipleVector(this->imgPlane.top, this->up)))); // q
    Vector3f topLeft = imageCenter + (this->right * this->imgPlane.left) + (this->up * this->imgPlane.top ); // q

    float i = (this->imgPlane.right - this->imgPlane.left) * (row + 0.5) / this->imgPlane.nx; // s_u
    float j = (this->imgPlane.top - this->imgPlane.bottom) * (col + 0.5) / this->imgPlane.ny; // s_v

    // {i, j, -this->imgPlane.distance}; // or z = this->gaze.z?
//    Vector3f rayDirection = vectorSubtract(
//            vectorSum(topLeft,
//                    vectorSubtract(
//                            scalarMultipleVector(i, this->right),
//                            scalarMultipleVector(j, this->up))), origin);
    Vector3f rayDirection = topLeft + (this->right * i) - (this->up * j) - origin;

    Ray * ray = new Ray(this->pos, rayDirection);
	normalize(ray->direction); // We have to normalize the direction to the length of 1 so it doesn't skew our results

    return *ray;
}

