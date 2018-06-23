#pragma once

#include "als_math.h"
#include "Entity.h"

struct Aabb;

struct RaycastResult
{
	Entity hitEntity;
	float32 t;
	bool hit;
};

struct Ray
{
	Ray() = default;
	Ray(Vec3 position, Vec3 direction);
	
	Vec3 position;
	Vec3 direction;
};

float32 rayPlaneTest(Ray ray, Plane plane);
float32 rayAabbTest(Ray ray, Aabb aabb);
