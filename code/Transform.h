#pragma once

#include "als_math.h"

struct Transform
{
	Vec3 position;
	Quaternion orientation;
	Vec3 scale;

	Transform();
	Transform(Vec3 position);
	Transform(Vec3 position, Quaternion orientation);
	Transform(Vec3 position, Quaternion orientation, Vec3 scale);

	inline Vec3 left()
	{
		Vec3 result = this->orientation * Vec3(-1, 0, 0);
		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}

	inline Vec3 right()
	{
		Vec3 result = this->orientation * Vec3(1, 0, 0);
		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}

	inline Vec3 up()
	{
		Vec3 result = this->orientation * Vec3(0, 1, 0);
		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}

	inline Vec3 down()
	{
		Vec3 result = this->orientation * Vec3(0, -1, 0);
		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}

    inline Vec3 forward()
    {
        Vec3 result = this->orientation * Vec3(0, 0, -1);
        assert(FLOAT_EQ(length(result), 1, EPSILON));
        return result;
    }

	inline Vec3 back()
	{
		Vec3 result = this->orientation * Vec3(0, 0, 1);
		assert(FLOAT_EQ(length(result), 1, EPSILON));
		return result;
	}
};

Mat4 modelToWorld(Transform* objectXfm);
Mat4 worldToView(Transform* cameraXfm);
