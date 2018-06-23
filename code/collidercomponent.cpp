#include "ColliderComponent.h"
#include <algorithm>

#include "TransformComponent.h"
#include "Ecs.h"

ColliderComponent::ColliderComponent()
{
    this->xfmOffset = Vec3(0, 0, 0);
    this->type = ColliderType::RECT3;
    this->xLength = 1;
    this->yLength = 1;
    this->zLength = 1;
}

ColliderComponent::ColliderComponent(Aabb aabb)
{
    this->xfmOffset = Vec3(0, 0, 0);
    this->type = ColliderType::RECT3;
    this->xLength = aabb.halfDim.x * 2;
    this->yLength = aabb.halfDim.y * 2;
    this->zLength = aabb.halfDim.z * 2;
}

Vec3 scaledXfmOffset(ColliderComponent* collider)
{
    TransformComponent *xfm = getTransformComponent(collider->entity);
    return hadamard(collider->xfmOffset, xfm->scale);
}

Vec3 colliderCenter(ColliderComponent* collider)
{
    TransformComponent *xfm = getTransformComponent(collider->entity);
    Vec3 unrotatedOffset = scaledXfmOffset(collider);
    Vec3 rotatedOffset = xfm->orientation * unrotatedOffset;
    return xfm->position + rotatedOffset;
}

real32 scaledLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE);
    
    TransformComponent *xfm = getTransformComponent(collider->entity);
    
    real32 scaleValue = getValue(xfm->scale, collider->axis);
    return collider->length * scaleValue;
}

real32 scaledRadius(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE || collider->type == ColliderType::SPHERE);

    TransformComponent *xfm = getTransformComponent(collider->entity);
    real32 scaleValue = std::max(std::max(xfm->scale.x, xfm->scale.y), xfm->scale.z);

    return scaleValue * collider->radius;
}

real32 scaledXLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getTransformComponent(collider->entity);
    return xfm->scale.x * collider->xLength;
}

real32 scaledYLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getTransformComponent(collider->entity);
    return xfm->scale.y * collider->yLength;
}

real32 scaledZLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getTransformComponent(collider->entity);
    return xfm->scale.z * collider->zLength;
}

bool pointInsideCollider(ColliderComponent* collider, Vec3 point)
{
    Quaternion orientation = getTransformComponent(collider->entity)->orientation;
    
    Vec3 center = colliderCenter(collider);

    // Optimize: this calculation doesn't need to be done for the SPHERE case
    Vec3 localX = orientation * Vec3(Axis3D::X);
    Vec3 localY = orientation * Vec3(Axis3D::Y);
    Vec3 localZ = orientation * Vec3(Axis3D::Z);

    switch(collider->type)
    {
        case RECT3:
        {
            real32 xLen = scaledXLength(collider);
            real32 yLen = scaledYLength(collider);
            real32 zLen = scaledZLength(collider);

            //   a
            //    |
            //    |  b
            //    | /
            //    |/
            //    -------- c
            // corner
                     
            Vec3 corner = center -
                (xLen / 2) * localX -
                (yLen / 2) * localY -
                (zLen / 2) * localZ;

            Vec3 a = center +
                (xLen / 2) * localX -
                (yLen / 2) * localY -
                (zLen / 2) * localZ;      

            Vec3 b = center -
                (xLen / 2) * localX +
                (yLen / 2) * localY -
                (zLen / 2) * localZ;

            Vec3 c = center -
                (xLen / 2) * localX -
                (yLen / 2) * localY +
                (zLen / 2) * localZ;

            Vec3 edge1 = a - corner;
            Vec3 edge2 = b - corner;
            Vec3 edge3 = c - corner;


            if (isBetween(dot(point, edge1), dot(corner, edge1), dot(a, edge1)))
            {
                if (isBetween(dot(point, edge2), dot(corner, edge2), dot(b, edge2)))
                {
                    if (isBetween(dot(point, edge3), dot(corner, edge3), dot(c, edge3)))
                    {
                        return true;
                    }
                }
            }

            return false;
            
        } break;

        case SPHERE:
        {
            Vec3 deltaPos = center - point;
            
            real32 radius = scaledRadius(collider);
            real32 radiusSquared = radius * radius;
            
            return (lengthSquared(deltaPos) < radiusSquared);
        } break;

        case CYLINDER:
        {
        } break;

        case CAPSULE:
        {
        } break;

        default:
        {
            assert(false);
        }
    }

    return false;
}
