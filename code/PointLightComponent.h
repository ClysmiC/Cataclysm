#pragma once

#include "Component.h"
#include "als_math.h"

struct PointLightComponent : public Component
{
    Vec3 intensity;

    float attenuationConstant  = 1.0f;
    float attenuationLinear    = 0.04f;
    float attenuationQuadratic = 0.01f;
};
