#include "ConvexHullColliderComponent.h"

Vec3 ConvexHullColliderComponent::center()
{
    static bool set = false;

    if (!set)
    {
        this->_colliderCenter = approximateHullCentroid(this); // Note: this assumes the hull never changes (it shouldn't)
    }
    
    return this->_colliderCenter;
}

Vec3 ConvexHullColliderComponent::support(Vec3 direction)
{
    Vec3 result;
    float32 biggestDot = -FLT_MAX;

    for (Vec3 position: this->positions)
    {
        float32 dotp = dot(position, direction);
        if (dotp > biggestDot)
        {
            result = position;
            biggestDot = dotp;
        }
    }

    return result;
}
