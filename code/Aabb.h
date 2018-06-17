#pragma once

#include "als_math.h"
#include "Transform.h"

struct Aabb
{
	Vec3 center;
	Vec3 halfDim;
};

Aabb transformedAabb(Aabb aabb, Transform* xfm);
