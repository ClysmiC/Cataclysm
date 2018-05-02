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

void TransformComponent::setPosition(Vec3 position) { position_ = position; cacheValid = false; }
void TransformComponent::setOrientation(Quaternion orientation) { orientation_ = orientation; cacheValid = false; }
void TransformComponent::setScale(Vec3 scale) { scale_ = scale; cacheValid = false; }

Mat4 TransformComponent::matrix()
{
	if (cacheValid) return matrix_;

	Mat4 result;
	result.translateInPlace(position_);
	result.rotateInPlace(orientation_);
	result.scaleInPlace(scale_);

	matrix_ = result;
	cacheValid = true;
	
	return matrix_;
}
