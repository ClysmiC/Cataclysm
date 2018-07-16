#pragma once

#include "Component.h"
#include "als_math.h"
#include "Aabb.h"
#include <string>

enum class ColliderType : uint32
{
    RECT3,
    SPHERE,
    CYLINDER,
    CAPSULE,

    ENUM_VALUE_COUNT
};

struct ColliderComponent : public Component
{
    ColliderComponent();
    ColliderComponent(Aabb aabb);
    
    Vec3 xfmOffset;
    ColliderType type;

    union
    {
        // Sphere, Cylinder, and Capsule types
        struct
        {
            float32 length; // unused for sphere
            Axis3D axis;    // unused for sphere
            
            float32 radius;
        };

        struct
        {
            Vec3 rect3Lengths;
        };
    };
};

Vec3 colliderCenter(ColliderComponent* collider);
Vec3 scaledXfmOffset(ColliderComponent* collider);
float32 scaledLength(ColliderComponent* collider);
float32 scaledRadius(ColliderComponent* collider);

Vec3 scaledRect3Lengths(ColliderComponent* collider);
float32 scaledXLength(ColliderComponent* collider);
float32 scaledYLength(ColliderComponent* collider);
float32 scaledZLength(ColliderComponent* collider);

bool pointInsideCollider(ColliderComponent* collider, Vec3 point);

