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

	real32 distance(TransformComponent* other);
	
	void setPosition(Vec3 pos);
	void setPosition(real32 x, real32 y, real32 z);
	void setOrientation(Quaternion orientation);
	void setScale(Vec3 scale);
	void setScale(real32 uniformScale);
	void setScale(real32 x, real32 y, real32 z);
	
	Mat4 matrix();

private:
	Mat4 matrix_;
	Vec3 position_;
	Quaternion orientation_;
	Vec3 scale_;
	bool cacheValid;
};
