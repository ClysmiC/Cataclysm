#include "Camera.h"

Mat4 calculateWorldToViewMatrix(TransformComponent* cameraXfm)
{
	Vec3 up = cameraXfm->up();
	Vec3 right = cameraXfm->right();
	Vec3 direction = -cameraXfm->forward(); // NOTE: This is direction pointing TOWARDS the camera (i.e., -forward)

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
	
	// Translate
	Mat4 translation = translationMatrix(-cameraXfm->position());
	result = result * translation;

	return result;
}
