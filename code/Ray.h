#pragma once

#include "als/als_math.h"
#include "ecs/Entity.h"

struct Aabb;
struct ColliderComponent;

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

float32 rayVsPlaneOneSided(Ray ray, Plane plane);
float32 rayVsPlaneTwoSided(Ray ray, Plane plane);
float32 rayVsRect3(Ray ray, Vec3 center, Quaternion orientation, Vec3 halfDim);
float32 rayVsCollider(Ray ray, ColliderComponent* collider);
float32 rayVsAabb(Ray ray, Aabb aabb);

inline Vec3 pointOnRay(Ray ray, float32 t)
{
    return ray.position + t * ray.direction;
}
