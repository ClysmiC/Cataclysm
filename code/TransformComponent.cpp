#include "TransformComponent.h"

TransformComponent::TransformComponent()
	: TransformComponent(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position)
	: TransformComponent(position, Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation)
	: TransformComponent(position, orientation, Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale)
	: position_(position), orientation_(orientation), scale_(scale)
{
}

Mat4 modelToWorld(TransformComponent* objectXfm)
{
	Mat4 result;
	result.scaleInPlace(objectXfm->scale);
	result.rotateInPlace(objectXfm->orientation);
	result.translateInPlace(objectXfm->position);
	
	return result;
}

Mat4 worldToView(TransformComponent* cameraXfm)
{
	Vec3 up = cameraXfm->up();
	Vec3 right = cameraXfm->right();
	Vec3 back = cameraXfm->back(); // NOTE: This is direction pointing TOWARDS the camera (i.e., -forward)

	Mat4 result;
	
	// Rotate
	result[0][0] = right.x;
	result[1][0] = up.x;
	result[2][0] = back.x;

	result[0][1] = right.y;
	result[1][1] = up.y;
	result[2][1] = back.y;
		
	result[0][2] = right.z;
	result[1][2] = up.z;
	result[2][2] = back.z;

	// Homogeneous
	result[3][3] = 1;
	
	// Translate
	Mat4 translation = translationMatrix(-cameraXfm->position);
	result = result * translation;

	return result;
}
