#pragma once

#include "als_math.h"
#include "Component.h"
#include "Transform.h"
#include <vector>

struct TransformComponent : public Component, public Transform
{
    Entity parent;
    std::vector<Entity> children;
    
    TransformComponent();
    TransformComponent(Vec3 position);
    TransformComponent(Vec3 position, Quaternion orientation);
    TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale);
    TransformComponent(const LiteTransform& transform);

    virtual Transform* getParent() override;
    virtual std::vector<Transform*> getChildren() override;
};

