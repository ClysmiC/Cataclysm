#pragma once

#include "Component.h"
#include "als_math.h"

struct DirectionalLightComponent : public Component
{
    Vec3 intensity;
    Vec3 direction;
};
