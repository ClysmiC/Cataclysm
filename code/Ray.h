#pragma once

#include "als_math.h"
#include "Entity.h"

struct Aabb;

struct RaycastResult
{
	Entity hitEntity;
	real32 t;
	bool hit;
};

struct Ray
{
	Ray() = default;
	Ray(Vec3 position, Vec3 direction);
	
	Vec3 position;
	Vec3 direction;
};

real32 rayPlaneTest(Ray ray, Plane plane);
real32 rayAabbTest(Ray ray, Aabb aabb);
