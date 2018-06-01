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

Vec3 TransformComponent::position() { return position_; }
Quaternion TransformComponent::orientation() { return orientation_; }
Vec3 TransformComponent::scale() { return scale_; }

real32
TransformComponent::distance(TransformComponent* other)
{
	return ::distance(position_, other->position());
}

void TransformComponent::setPosition(Vec3 position)
{
	position_ = position;
	m2wCacheValid = false;
}

void TransformComponent::setPosition(real32 x, real32 y, real32 z)
{
	position_.x = x;
	position_.y = y;
	position_.z = z;
	m2wCacheValid = false;
}

void TransformComponent::setOrientation(Quaternion orientation)
{
	orientation_ = orientation;
	m2wCacheValid = false;
}

void TransformComponent::setScale(Vec3 scale)
{
	scale_ = scale;
	m2wCacheValid = false;
}

void TransformComponent::setScale(real32 uniformScale)
{
	scale_ = Vec3(uniformScale);
	m2wCacheValid = false;
}

void TransformComponent::setScale(real32 x, real32 y, real32 z)
{
	scale_ = Vec3(x, y, z);
	m2wCacheValid = false;
}

Mat4 TransformComponent::modelToWorld()
{
	if (m2wCacheValid) return modelToWorldMatrix_;

	Mat4 result;
	result.scaleInPlace(scale_);
	result.rotateInPlace(orientation_);
	result.translateInPlace(position_);

	modelToWorldMatrix_ = result;
	m2wCacheValid = true;
	
	return modelToWorldMatrix_;
}

Mat4
TransformComponent::worldToView()
{
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
	Mat4 translation = translationMatrix(-position_);
	result = result * translation;

	return result;
}
