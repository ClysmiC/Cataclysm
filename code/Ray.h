#pragma once

#include "als_math.h"
// #include "Aabb.h"

struct Aabb;

struct RaycastResult
{
	bool hit;
	real32 t;
};

struct Ray
{
	Ray() = default;
	Ray(Vec3 position, Vec3 direction);
	
	Vec3 position;
	Vec3 direction;
};

RaycastResult rayPlaneTest(Ray ray, Plane plane);
RaycastResult rayAabbTest(Ray ray, Aabb aabb);
