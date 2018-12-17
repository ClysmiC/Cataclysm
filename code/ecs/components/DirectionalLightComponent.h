#pragma once

#include "ecs/IComponent.h"
#include "als/als_math.h"

struct DirectionalLightComponent : public IComponent
{
    Vec3 intensity;
    Vec3 direction;

    static constexpr bool multipleAllowedPerEntity = true;
};
