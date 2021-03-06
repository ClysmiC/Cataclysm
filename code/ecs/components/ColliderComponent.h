#pragma once

#include "ICollider.h"
#include "ecs/IComponent.h"
#include "als/als_math.h"

struct Aabb;

enum class ColliderType : uint32
{
    RECT3,
    SPHERE,
    CYLINDER,
    CAPSULE,

    ENUM_VALUE_COUNT
};

struct ColliderComponent : public IComponent, public ICollider
{
    ColliderComponent();
    ColliderComponent(Entity e, Aabb aabb);
    
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

    static constexpr bool multipleAllowedPerEntity = true;

    Vec3 center() override;
    Vec3 support(Vec3 direction) override;
};

Vec3 scaledXfmOffset(ColliderComponent* collider);
float32 scaledLength(ColliderComponent* collider);
float32 scaledRadius(ColliderComponent* collider);

Vec3 scaledRect3Lengths(ColliderComponent* collider);
float32 scaledXLength(ColliderComponent* collider);
float32 scaledYLength(ColliderComponent* collider);
float32 scaledZLength(ColliderComponent* collider);

bool pointInsideCollider(ColliderComponent* collider, Vec3 point);

