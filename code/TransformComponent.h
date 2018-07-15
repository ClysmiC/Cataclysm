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

    Vec3 localPosition();
    Quaternion localOrientation();
    Vec3 localScale();

    void setLocalPosition(Vec3 localPosition);
    void setLocalOrientation(Quaternion localOrientation);
    void setLocalScale(Vec3 localScale);

    void resetLocal();

    bool hasParentTransform();
    
    Entity parent;
};


