#pragma once

#include "als_math.h"
#include "Component.h"
#include "Transform.h"

struct TransformComponent : public Component, public Transform
{
    TransformComponent();
    TransformComponent(Vec3 position);
    TransformComponent(Vec3 position, Quaternion orientation);
    TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale);
};

