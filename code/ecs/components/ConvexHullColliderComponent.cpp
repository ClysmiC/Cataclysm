#include "ConvexHullColliderComponent.h"

#include "ecs/Ecs.h"
#include "ecs/components/TransformComponent.h"

Vec3 ConvexHullColliderComponent::center()
{
    if (!_centerCalculated)
    {
        this->_centerCalculated = true;
        this->_colliderCenter = approximateHullCentroid(this); // Note: this assumes the hull never changes (it shouldn't)
    }
    
    return this->_colliderCenter;
}

Vec3 ConvexHullColliderComponent::support(Vec3 direction)
{
    TransformComponent* xfm = getComponent<TransformComponent>(this->entity);
    Vec3 result;
    float32 biggestDot = -FLT_MAX;

    for (Vec3 position: this->positions)
    {
        Vec3 worldPosition = xfm->position() + position;

        float32 dotp = dot(worldPosition, direction);
        if (dotp > biggestDot)
        {
            result = worldPosition;
            biggestDot = dotp;
        }
    }

    return result;
}
