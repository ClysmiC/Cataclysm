#include "Camera.h"

Camera::Camera()
{
	// position = Vec3(0, 0, 0);
	// orientation = Vec4(0, 0, 0, 1);
}

Camera::~Camera() {}

Mat4 Camera::worldToView()
{
	Vec3 up = this->up();
	Vec3 right = this->right();
	Vec3 direction = -this->forward(); // NOTE: This is direction pointing TOWARDS the camera (i.e., -forward)

	Mat4 result;
	
	// Rotate
	result[0][0] = right.x;
	result[1][0] = up.x;
	result[2][0] = direction.x;

	result[0][1] = right.y;
	result[1][1] = up.y;
	result[2][1] = direction.y;
		
	result[0][2] = right.z;
	result[1][2] = up.z;
	result[2][2] = direction.z;

	// Homogeneous
	result[3][3] = 1;

	Vec3 position(0, 0, 0); // TEMPORARY
	
	// Translate
	Mat4 translation = translationMatrix(-position);
	result = result * translation;

	return result;
}
