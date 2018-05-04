#pragma once

#include "Component.h"
#include "als_math.h"

struct PointLightComponent : public Component
{
	Vec3 intensity;
};
