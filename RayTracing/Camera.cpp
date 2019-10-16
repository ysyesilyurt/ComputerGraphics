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
}

