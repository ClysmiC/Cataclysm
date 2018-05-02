#pragma once

#include "als_math.h"
#include "Component.h"

struct TransformComponent : public Component
{
	TransformComponent();
	TransformComponent(Vec3 position);
	TransformComponent(Vec3 position, Quaternion orientation);
	TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale);
	
	~TransformComponent() = default;
	
	// Note: transform members are wrapped in getters/setters so that we can cache
	// the transformation matrix and invalidate the cached matrix in the setters
	Vec3 position();
	Quaternion orientation();
	Vec3 scale();
	
	void setPosition(Vec3 pos);
	void setOrientation(Quaternion orientation);
	void setScale(Vec3 scale);
	
	Mat4 matrix();

private:
	Mat4 matrix_;
	Vec3 position_;
	Quaternion orientation_;
	Vec3 scale_;
	bool cacheValid;
};
