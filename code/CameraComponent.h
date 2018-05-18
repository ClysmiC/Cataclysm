#pragma once

#include "Component.h"

#include "als_math.h"
#include "TransformComponent.h"

struct CameraComponent : public Component
{
	Mat4 projectionMatrix;
	TransformComponent* cachedXfm = nullptr;
	bool isOrthographic;

	// TODO: cache this if camera hasnt moved (then return pointer to cached value)
	Mat4 worldToViewMatrix();

	Vec3 right();
	Vec3 left();
	Vec3 up();
	Vec3 down();
	Vec3 forward();
	Vec3 back();

private:
	void setTransformComponentIfNeeded();
};



