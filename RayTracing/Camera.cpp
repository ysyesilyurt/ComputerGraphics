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
    this->right = up * -1;
    this->up = crossProduct(gaze, right);
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

    Vector3f origin = this->pos; // e
    Vector3f imageCenter = origin + (this->gaze * this->imgPlane.distance); // m
    Vector3f topLeft = imageCenter + (this->right * this->imgPlane.left) + (this->up * this->imgPlane.top ); // q
    float i = (this->imgPlane.right - this->imgPlane.left) * (row + 0.5) / this->imgPlane.nx; // s_u
    float j = (this->imgPlane.top - this->imgPlane.bottom) * (col + 0.5) / this->imgPlane.ny; // s_v

    // {i, j, -this->imgPlane.distance}; // or z = this->gaze.z?

    // We have to normalize the direction to the length of 1 so it doesn't skew our results
    Vector3f rayDirection = (topLeft + (this->right * i) - (this->up * j) - origin).normalize(); // d = s - e

    Ray * ray = new Ray(this->pos, rayDirection);
    return *ray;
}

