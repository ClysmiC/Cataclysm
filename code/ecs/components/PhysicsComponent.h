#pragma once

#include "als/als_math.h"
#include "ecs/Component.h"

struct PhysicsComponent : public Component
{
    Vec3 velocity;

    static constexpr bool multipleAllowedPerEntity = false;
};
