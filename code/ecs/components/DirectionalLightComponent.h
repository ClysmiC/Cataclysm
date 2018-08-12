#pragma once

#include "ecs/Component.h"
#include "als/als_math.h"

struct DirectionalLightComponent : public Component
{
    Vec3 intensity;
    Vec3 direction;

    static constexpr bool multipleAllowedPerEntity = true;
};
