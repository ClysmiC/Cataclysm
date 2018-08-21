#include "ecs/Component.h"
#include "ConvexHull.h"

struct ConvexHullColliderComponent : public Component, public ConvexHull
{
};

inline void stdmoveConvexHullIntoComponent(ConvexHullColliderComponent* component, ConvexHull* hull)
{
    component->positions = std::move(hull->positions);
    component->edges = std::move(hull->edges);
};
