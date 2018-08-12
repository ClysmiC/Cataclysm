#pragma once

#include "als/als_math.h"
#include "ecs/ComponentGroup.h"

struct ITransform;
struct RenderComponent;
struct ColliderComponent;

struct Aabb
{
    Aabb() = default;
    Aabb(Vec3 center, Vec3 halfDim);
    
    Vec3 center;
    Vec3 halfDim;

    inline Vec3 minPoint() { return center - halfDim; }
    inline Vec3 maxPoint() { return center + halfDim; }
};


Aabb transformedAabb(Aabb aabb, ITransform* xfm);
Aabb aabbFromMinMax(Vec3 min, Vec3 max);
Aabb aabbFromRenderComponent(RenderComponent* rc);
Aabb aabbFromCollider(ColliderComponent* collider);

template <uint32 BUCKET_SIZE>
Aabb aabbFromRenderComponents(ComponentGroup<RenderComponent, BUCKET_SIZE> rcg) // TODO: change this param to pointer
{
    Vec3 minPoint = Vec3(FLT_MAX);
    Vec3 maxPoint = Vec3(-FLT_MAX);
    
    for (uint32 i = 0; i < rcg.numComponents; i++)
    {
        RenderComponent* it = &rcg[i];
        Aabb bounds = aabbFromRenderComponent(it);
        Vec3 boundsMin = bounds.center - bounds.halfDim;
        Vec3 boundsMax = bounds.center + bounds.halfDim;

        minPoint.x = min(minPoint.x, boundsMin.x);
        minPoint.y = min(minPoint.y, boundsMin.y);
        minPoint.z = min(minPoint.z, boundsMin.z);
            
        maxPoint.x = max(maxPoint.x, boundsMax.x);
        maxPoint.y = max(maxPoint.y, boundsMax.y);
        maxPoint.z = max(maxPoint.z, boundsMax.z);
    }

    Vec3 aabbCenter = (maxPoint + minPoint) / 2;
    Vec3 halfDims = (maxPoint - minPoint) / 2;

    Aabb result = Aabb(aabbCenter, halfDims);
    return result;
}

template <uint32 BUCKET_SIZE>
Aabb aabbFromColliders(ComponentGroup<ColliderComponent, BUCKET_SIZE> colliders)
{
    Vec3 minPoint = Vec3(FLT_MAX);
    Vec3 maxPoint = Vec3(-FLT_MAX);

    for (uint32 i = 0; i < colliders.numComponents; i++)    
    {
        ColliderComponent* it = &colliders[i];

        Aabb bounds = aabbFromCollider(it);
        Vec3 boundsMin = bounds.center - bounds.halfDim;
        Vec3 boundsMax = bounds.center + bounds.halfDim;

        minPoint.x = min(minPoint.x, boundsMin.x);
        minPoint.y = min(minPoint.y, boundsMin.y);
        minPoint.z = min(minPoint.z, boundsMin.z);
            
        maxPoint.x = max(maxPoint.x, boundsMax.x);
        maxPoint.y = max(maxPoint.y, boundsMax.y);
        maxPoint.z = max(maxPoint.z, boundsMax.z);
    }

    Aabb result = aabbFromMinMax(minPoint, maxPoint);
    return result;
}
