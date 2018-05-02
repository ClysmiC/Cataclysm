#pragma once

#include "Entity.h"
#include "als_math.h"


class Camera : public Entity
{
public:
	Camera();
	~Camera();

	// TODO: cache/precompute this
	Mat4 worldToView();

	inline Vec3 forward()
	{
		// NOTE: Default orientation is looking down the -Z axis
		Vec3 result = orientation * Vec3(0, 0, -1);

		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}

	inline Vec3 right()
	{
		Vec3 result = orientation * Vec3(1, 0, 0);

		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}

	inline Vec3 up()
	{
		Vec3 result = orientation * Vec3(0, 1, 0);

		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}
};
