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

	inline Vec3 right()
	{
		setTransformComponentIfNeeded();
		return cachedXfm->orientation() * Vec3(1, 0, 0);
	}
	
	inline Vec3 left()
	{
		return -right();
	}

	inline Vec3 up()
	{
		setTransformComponentIfNeeded();
		return cachedXfm->orientation() * Vec3(0, 1, 0);
	}

	inline Vec3 down()
	{
		return -up();
	}

	inline Vec3 forward()
	{
		setTransformComponentIfNeeded();
		return cachedXfm->orientation() * Vec3(0, 0, -1);
	}

	inline Vec3 back()
	{
		return -forward();
	}

private:
	inline void setTransformComponentIfNeeded()
	{
		assert(entity.id != 0);
		if (cachedXfm == nullptr)
		{
			cachedXfm = ecs->getTransformComponent(this);
		}
	}
};



