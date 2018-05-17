#include "CameraComponent.h"

Mat4
CameraComponent::worldToViewMatrix()
{
	setTransformComponentIfNeeded();
	
	Vec3 up_ = up();
	Vec3 right_ = right();
	Vec3 back_ = back(); // NOTE: This is direction pointing TOWARDS the camera (i.e., -forward)

	Mat4 result;
	
	// Rotate
	result[0][0] = right_.x;
	result[1][0] = up_.x;
	result[2][0] = back_.x;

	result[0][1] = right_.y;
	result[1][1] = up_.y;
	result[2][1] = back_.y;
		
	result[0][2] = right_.z;
	result[1][2] = up_.z;
	result[2][2] = back_.z;

	// Homogeneous
	result[3][3] = 1;
	
	// Translate
	Mat4 translation = translationMatrix(-cachedXfm->position());
	result = result * translation;

	return result;
}
