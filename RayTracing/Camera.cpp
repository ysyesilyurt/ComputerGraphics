#include <cstring>
#include "Camera.h"

Camera::Camera(int id,                      // Id of the camera
               const char* imageName,       // Name of the output PPM file 
               const Vector3f& pos,         // Camera position
               const Vector3f& gaze,        // Camera gaze direction
               const Vector3f& up,          // Camera up direction
               const ImagePlane& imgPlane)  // Image plane parameters
{
	/**********************************************
     *                                             *
	* TODO: Implement this function               *
     *                                             *
     ***********************************************
     ADD up,pos,gaze as variables to private section
     Map everything
	 */
	 this->id = id;
	 strncpy(this->imageName, imageName, 32);
	 this->imgPlane = imgPlane;
	 this->pos = pos;
	 this->gaze = gaze;
	 this->up = up;
}

/* Takes coordinate of an image pixel as row and col, and
 * returns the ray going through that pixel. 
 */
Ray Camera::getPrimaryRay(int col, int row) const
{
	/**********************************************
     *                                             *
	* TODO: Implement this function               *
     *                                             *
     ***********************************************
     
     Calculate the ray parameters based on pixel, e and gaze
     Create the ray instance
     Return the ray through point
	 */
	 float i = (this->imgPlane.left - this->imgPlane.right) * (row + 0.5) / this->imgPlane.nx;
	 float j = (this->imgPlane.top - this->imgPlane.bottom) * (col + 0.5) / this->imgPlane.ny;
	 Vector3f direction = {i, j, -this->imgPlane.distance}; // or z = this->gaze.z? CHECK FROM other hws
	 Ray * ray = new Ray(this->pos, direction);
	 return *ray;
}

