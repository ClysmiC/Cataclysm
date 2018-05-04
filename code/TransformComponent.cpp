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
	cacheValid = false;
}

void TransformComponent::setPosition(real32 x, real32 y, real32 z)
{
	position_.x = x;
	position_.y = y;
	position_.z = z;
	cacheValid = false;
}

void TransformComponent::setOrientation(Quaternion orientation)
{
	orientation_ = orientation;
	cacheValid = false;
}

void TransformComponent::setScale(Vec3 scale)
{
	scale_ = scale;
	cacheValid = false;
}

void TransformComponent::setScale(real32 uniformScale)
{
	scale_ = Vec3(uniformScale);
	cacheValid = false;
}

void TransformComponent::setScale(real32 x, real32 y, real32 z)
{
	scale_ = Vec3(x, y, z);
	cacheValid = false;
}

Mat4 TransformComponent::matrix()
{
	if (cacheValid) return matrix_;

	Mat4 result;
	result.scaleInPlace(scale_);
	result.rotateInPlace(orientation_);
	result.translateInPlace(position_);

	matrix_ = result;
	cacheValid = true;
	
	return matrix_;
}
