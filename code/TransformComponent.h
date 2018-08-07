#pragma once

#include "als_math.h"
#include "Component.h"
#include "Transform.h"
#include <vector>

struct TransformComponent : public Component, public ITransform
{
    TransformComponent();
    TransformComponent(Vec3 position);
    TransformComponent(Vec3 position, Quaternion orientation);
    TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale);
    TransformComponent(ITransform& transform);

    virtual ITransform* getParent() override;
    virtual std::vector<ITransform*> getChildren() override;

    static constexpr bool multipleAllowedPerEntity = false;
};
