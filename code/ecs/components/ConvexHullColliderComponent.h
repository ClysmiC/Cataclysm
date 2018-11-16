#pragma once

#include "ecs/Component.h"
#include "ConvexHull.h"
#include "ICollider.h"
#include <utility>

struct ConvexHullColliderComponent : public Component, public ConvexHull, public ICollider
{
    ConvexHullColliderComponent() = default;

    static constexpr bool multipleAllowedPerEntity = true;

    bool showInEditor = true;

    bool _centerCalculated;
    Vec3 _colliderCenter;

    Vec3 center() override;
    Vec3 support(Vec3 direction) override;
};

inline void stdmoveConvexHullIntoComponent(ConvexHullColliderComponent* component, ConvexHull* hull)
{
    component->positions = std::move(hull->positions);
    component->edges = std::move(hull->edges);
    component->bounds = hull->bounds;
};
