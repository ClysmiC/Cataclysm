#pragma once

#include "Component.h"
#include "als_math.h"
#include "Aabb.h"

enum ColliderType : int32
{
    RECT3,
    SPHERE,
    CYLINDER,
    CAPSULE,
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
            float32 radius;
            Axis3D axis; 
        };

        struct
        {
            float32 xLength; // use 'length' to address this memory
            float32 yLength;
            float32 zLength;
        };
    };
};

Vec3 colliderCenter(ColliderComponent* collider);
Vec3 scaledXfmOffset(ColliderComponent* collider);
float32 scaledLength(ColliderComponent* collider);
float32 scaledRadius(ColliderComponent* collider);

float32 scaledXLength(ColliderComponent* collider);
float32 scaledYLength(ColliderComponent* collider);
float32 scaledZLength(ColliderComponent* collider);

bool pointInsideCollider(ColliderComponent* collider, Vec3 point);

