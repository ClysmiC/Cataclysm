#include "Aabb.h"
#include "float.h"
#include <algorithm>

#include "Transform.h"

#include "ecs/Ecs.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/ColliderComponent.h"
#include "ecs/components/ConvexHullColliderComponent.h"
#include "ecs/components/TransformComponent.h"
#include "resource/Submesh.h"
#include "resource/resources/Mesh.h"

Aabb::Aabb(Vec3 center, Vec3 halfDim)
{
    this->center = center;
    this->halfDim = halfDim;
}

Aabb transformedAabb(Aabb aabb, ITransform* xfm)
{
    //
    // Scale
    //
    Aabb result;
    result.center = hadamard(aabb.center, xfm->scale());
    result.halfDim = hadamard(aabb.halfDim, xfm->scale());
    
    float32 minX = FLT_MAX;
    float32 minY = FLT_MAX;
    float32 minZ = FLT_MAX;

    float32 maxX = -FLT_MAX;
    float32 maxY = -FLT_MAX;
    float32 maxZ = -FLT_MAX;

    //
    // Resize based on rotate
    //
    for (uint32 i = 0; i < 8; i++)
    {
        // Iterate over every combination of ( center +/- halfDim.x, center +/- halfDim.y, center +/- halfDim.z )

        Vec3 corner = result.center +
            ((i & 1 << 0) ? 1 : -1) * result.halfDim.x * Vec3(Axis3D::X) +
            ((i & 1 << 1) ? 1 : -1) * result.halfDim.y * Vec3(Axis3D::Y) +
            ((i & 1 << 2) ? 1 : -1) * result.halfDim.z * Vec3(Axis3D::Z);

        Vec3 rotatedCorner = xfm->orientation() * corner;

        minX = std::min(minX, rotatedCorner.x);
        minY = std::min(minY, rotatedCorner.y);
        minZ = std::min(minZ, rotatedCorner.z);
        
        maxX = std::max(maxX, rotatedCorner.x);
        maxY = std::max(maxY, rotatedCorner.y);
        maxZ = std::max(maxZ, rotatedCorner.z);
    }

    Vec3 minPoint = Vec3(minX, minY, minZ);
    Vec3 maxPoint = Vec3(maxX, maxY, maxZ);

    result.halfDim = Vec3(
        (maxPoint.x - minPoint.x) / 2.0f,
        (maxPoint.y - minPoint.y) / 2.0f,
        (maxPoint.z - minPoint.z) / 2.0f
    );
    
    result.center = minPoint + result.halfDim;

    //
    // Translate
    //
    result.center += xfm->position();

    return result;
}

Aabb aabbFromMinMax(Vec3 min, Vec3 max)
{
    Aabb result((max + min) / 2, (max - min) / 2);
    return result;
}

Aabb aabbFromRenderComponent(RenderComponent* rc)
{
    TransformComponent* xfm = getTransformComponent(rc->entity);
    assert(xfm);
    
    return transformedAabb(rc->submesh->mesh->bounds, xfm);
}

Aabb aabbFromCollider(ColliderComponent* collider)
{
    TransformComponent* xfm = getTransformComponent(collider->entity);
    assert(xfm);

    switch (collider->type)
    {
        case ColliderType::RECT3:
        {
            Aabb unXfmedBounds(collider->xfmOffset, scaledRect3Lengths(collider) / 2);
            return transformedAabb(unXfmedBounds, xfm);
        } break;

        case ColliderType::SPHERE:
        {
            Aabb unXfmedBounds(collider->xfmOffset, Vec3(scaledRadius(collider)));
            return transformedAabb(unXfmedBounds, xfm);
        } break;

        case ColliderType::CYLINDER:
        {
            float32 xLen = 0, yLen = 0, zLen = 0;
            
            switch (collider->axis)
            {
                case Axis3D::X:
                {
                    xLen = scaledLength(collider);
                    yLen = zLen = scaledRadius(collider) * 2;
                } break;

                case Axis3D::Y:
                {
                    yLen = scaledLength(collider);
                    xLen = zLen = scaledRadius(collider) * 2;
                } break;

                case Axis3D::Z:
                {
                    zLen = scaledLength(collider);
                    xLen = yLen = scaledRadius(collider) * 2;
                } break;

                default: { assert(false); }
            }
            
            Aabb unXfmedBounds(collider->xfmOffset, Vec3(xLen, yLen, zLen) / 2);
            return transformedAabb(unXfmedBounds, xfm);
        } break;

        case ColliderType::CAPSULE:
        {
            float32 xLen = 0, yLen = 0, zLen = 0;
            
            switch (collider->axis)
            {
                case Axis3D::X:
                {
                    xLen = scaledLength(collider) + scaledRadius(collider);
                    yLen = zLen = scaledRadius(collider) * 2;
                } break;

                case Axis3D::Y:
                {
                    yLen = scaledLength(collider) + scaledRadius(collider);
                    xLen = zLen = scaledRadius(collider) * 2;
                } break;

                case Axis3D::Z:
                {
                    zLen = scaledLength(collider) + scaledRadius(collider);
                    xLen = yLen = scaledRadius(collider) * 2;
                } break;

                default: { assert(false); }
            }
            
            Aabb unXfmedBounds(collider->xfmOffset, Vec3(xLen, yLen, zLen) / 2);
            return transformedAabb(unXfmedBounds, xfm);
        } break;

        default:
        {
            assert(false);
        }
    }
    

    Aabb result = aabbFromMinMax(Vec3(0), Vec3(0));
    return result;
}

Aabb aabbFromConvexCollider(ConvexHullColliderComponent* collider)
{
    TransformComponent* xfm = getTransformComponent(collider->entity);
    Aabb result = transformedAabb(collider->bounds, xfm); // @Think: should we call recalculate bounds here? It would be slower, but could avoid bugs if the bounds were never calculated
    return result;
}

