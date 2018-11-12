#pragma once

#include "ecs/Component.h"
#include "ConvexHull.h"
#include <utility>

struct ConvexHullColliderComponent : public Component, public ConvexHull
{
    ConvexHullColliderComponent() = default;

    static constexpr bool multipleAllowedPerEntity = true;
};

inline void stdmoveConvexHullIntoComponent(ConvexHullColliderComponent* component, ConvexHull* hull)
{
    component->positions = std::move(hull->positions);
    component->edges = std::move(hull->edges);
    component->bounds = hull->bounds;
};
