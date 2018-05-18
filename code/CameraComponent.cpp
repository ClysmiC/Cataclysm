#include "CameraComponent.h"
#include "Ecs.h"

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

Vec3
CameraComponent::right()
{
	setTransformComponentIfNeeded();
	return cachedXfm->orientation() * Vec3(1, 0, 0);
}
	
Vec3
CameraComponent::left()
{
	return -right();
}

Vec3
CameraComponent::up()
{
	setTransformComponentIfNeeded();
	return cachedXfm->orientation() * Vec3(0, 1, 0);
}

Vec3
CameraComponent::down()
{
	return -up();
}

Vec3
CameraComponent::forward()
{
	setTransformComponentIfNeeded();
	return cachedXfm->orientation() * Vec3(0, 0, -1);
}

Vec3
CameraComponent::back()
{
	return -forward();
}

void
CameraComponent::setTransformComponentIfNeeded()
{
	assert(entity.id != 0);
	if (cachedXfm == nullptr)
	{
		cachedXfm = ecs->getTransformComponent(this->entity);
	}
}
