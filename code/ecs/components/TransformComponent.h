#pragma once

#include "als/als_math.h"
#include "ecs/IComponent.h"
#include "Transform.h"
#include <vector>

struct TransformComponent : public IComponent, public ITransform
{
    TransformComponent() = default;
    TransformComponent(Entity e);
    TransformComponent(Entity e, Vec3 position);
    TransformComponent(Entity e, Vec3 position, Quaternion orientation);
    TransformComponent(Entity e, Vec3 position, Quaternion orientation, Vec3 scale);
    TransformComponent(Entity e, ITransform& transform);

    virtual ITransform* getParent() override;
    virtual std::vector<ITransform*> getChildren() override;

    static constexpr bool multipleAllowedPerEntity = false;
};
