#pragma once

#include "Component.h"
#include "als_math.h"

struct PointLightComponent : public Component
{
    Vec3 intensity;

    float32 attenuationConstant  = 1.0f;
    float32 attenuationLinear    = 0.04f;
    float32 attenuationQuadratic = 0.01f;

    static constexpr bool multipleAllowedPerEntity = true;
};
