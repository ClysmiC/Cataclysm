#pragma once

#include "Entity.h"
#include "als_math.h"


struct Camera // : public Entity
{
	Camera();
	~Camera();

	// TODO: cache/precompute this
	Mat4 worldToView();

	// TODO: get forward / right / up from transform component... should we cache a pointer to it?
	// should we even have a class for camera? (probably) or should it and every other entity only exist
	// in the component tables and not have a sense of being their own class...

	inline Vec3 forward()
	{
		return Vec3(0, 0, -1);
		// // NOTE: Default orientation is looking down the -Z axis
		// Vec3 result = orientation * Vec3(0, 0, -1);

		// assert(FLOAT_EQ(length(result), 1, EPSILON));
		// return result;
	}

	inline Vec3 right()
	{
		return Vec3(1, 0, 0);
		// Vec3 result = orientation * Vec3(1, 0, 0);

		// assert(FLOAT_EQ(length(result), 1, EPSILON));
		// return result;
	}

	inline Vec3 up()
	{
		return Vec3(0, 1, 0);
		// Vec3 result = orientation * Vec3(0, 1, 0);

		// assert(FLOAT_EQ(length(result), 1, EPSILON));
		// return result;
	}
};
