#include "ColliderComponent.h"
#include <algorithm>

#include "TransformComponent.h"
#include "ecs/Ecs.h"

#include "Aabb.h"

ColliderComponent::ColliderComponent()
{
    this->xfmOffset = Vec3(0, 0, 0);
    this->type = ColliderType::RECT3;
    this->rect3Lengths.x = 1;
    this->rect3Lengths.y = 1;
    this->rect3Lengths.z = 1;
}

ColliderComponent::ColliderComponent(Entity e, Aabb aabb)
    : IComponent(e)
{
    this->xfmOffset = Vec3(0, 0, 0);
    this->type = ColliderType::RECT3;
    this->rect3Lengths.x = aabb.halfDim.x * 2;
    this->rect3Lengths.y = aabb.halfDim.y * 2;
    this->rect3Lengths.z = aabb.halfDim.z * 2;
}

Vec3 ColliderComponent::center()
{
    TransformComponent *xfm = getComponent<TransformComponent>(this->entity);
    Vec3 unrotatedOffset = scaledXfmOffset(this);
    Vec3 rotatedOffset = xfm->orientation() * unrotatedOffset;
    return xfm->position() + rotatedOffset;
}

Vec3 ColliderComponent::support(Vec3 direction)
{
    // Consider collider in identity position/orientation.
    // Need to rotate direction accordingly
    direction.normalizeInPlace();

    Quaternion orientation = getComponent<TransformComponent>(this->entity)->orientation();
    Quaternion toIdentity = relativeRotation(orientation, Quaternion());

    Vec3 relativeDirection = toIdentity * direction;
    Vec3 result; // After storing result here, we will rotate it back into the correct orientation
    
    switch (this->type)
    {
        case ColliderType::RECT3:
        {
            // Optimize: this calculation doesn't need to be done for the SPHERE case
            Vec3 xAxis = Vec3(Axis3D::X);
            Vec3 yAxis = Vec3(Axis3D::Y);
            Vec3 zAxis = Vec3(Axis3D::Z);

            float32 xLen = scaledXLength(this);
            float32 yLen = scaledYLength(this);
            float32 zLen = scaledZLength(this);
            //   a
            //    |
            //    |  b
            //    | /
            //    |/
            //    -------- c
            // corner
                     
            Vec3 corner =
                - (xLen / 2) * xAxis
                - (yLen / 2) * yAxis
                - (zLen / 2) * zAxis;

            Vec3 a =
                + (xLen / 2) * xAxis
                - (yLen / 2) * yAxis
                - (zLen / 2) * zAxis;      

            Vec3 b =
                - (xLen / 2) * xAxis
                + (yLen / 2) * yAxis
                - (zLen / 2) * zAxis;

            Vec3 c =
                - (xLen / 2) * xAxis
                - (yLen / 2) * yAxis
                + (zLen / 2) * zAxis;

            Vec3 edge1 = a - corner;
            Vec3 edge2 = b - corner;
            Vec3 edge3 = c - corner;

            float32 maxDot = dot(corner, relativeDirection);
            Vec3 maxDotVec = corner;

            for (int i = 1; i < 8; i++)
            {
                Vec3 point = corner;
                if ((i & 1) > 0) point += edge1;
                if ((i & 2) > 0) point += edge2;
                if ((i & 4) > 0) point += edge3;

                float32 dotVal = dot(point, relativeDirection);

                if (dotVal > maxDot)
                {
                    maxDot = dotVal;
                    maxDotVec = point;
                }
            }

            result = maxDotVec;
        } break;

        case ColliderType::SPHERE:
        {
            result = scaledRadius(this) * relativeDirection;
        } break;

        case ColliderType::CYLINDER:
        {            
            if (this->axis == Axis3D::X)
            {
                result.x = scaledLength(this) / 2 * (relativeDirection.x > 0 ? 1 : -1);

                Vec3 circlePart = normalize(Vec3(0, relativeDirection.y, relativeDirection.z));
                circlePart *= scaledRadius(this);
                result.y = circlePart.y;
                result.z = circlePart.z;
            }
            else if (this->axis == Axis3D::Y)
            {
                result.y = scaledLength(this) / 2 * (relativeDirection.y > 0 ? 1 : -1);

                Vec3 circlePart = normalize(Vec3(relativeDirection.x, 0, relativeDirection.z));
                circlePart *= scaledRadius(this);
                result.x = circlePart.x;
                result.z = circlePart.z;
            }
            else if (this->axis == Axis3D::Z)
            {
                result.z = scaledLength(this) / 2 * (relativeDirection.z > 0 ? 1 : -1);

                Vec3 circlePart = normalize(Vec3(relativeDirection.x, relativeDirection.y, 0));
                circlePart *= scaledRadius(this);
                result.x = circlePart.x;
                result.y = circlePart.y;
            }
            else assert(false);

        } break;

        case ColliderType::CAPSULE:
        {            
            if (this->axis == Axis3D::X)
            {
                Vec3 endpointSphereCenter = scaledLength(this) / 2 * Vec3(Axis3D::X) * (relativeDirection.x > 0 ? 1 : -1);
                result = endpointSphereCenter + normalize(relativeDirection) * scaledRadius(this);
            }
            else if (this->axis == Axis3D::Y)
            {
                Vec3 endpointSphereCenter = scaledLength(this) / 2 * Vec3(Axis3D::Y) * (relativeDirection.y > 0 ? 1 : -1);
                result = endpointSphereCenter + normalize(relativeDirection) * scaledRadius(this);
            }
            else if (this->axis == Axis3D::Z)
            {
                Vec3 endpointSphereCenter = scaledLength(this) / 2 * Vec3(Axis3D::Z) * (relativeDirection.z > 0 ? 1 : -1);
                result = endpointSphereCenter + normalize(relativeDirection) * scaledRadius(this);
            }
            else assert(false);

        } break;

        default:
        {
            assert(false);
        }
    }

    // Re-orient and offset
    result = orientation * result;
    result = this->center() + result;
    return result;
}

Vec3 scaledXfmOffset(ColliderComponent* collider)
{
    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    return hadamard(collider->xfmOffset, xfm->scale());
}

float32 scaledLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE);
    
    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    
    float32 scaleValue = getValue(xfm->scale(), collider->axis);
    return collider->length * scaleValue;
}

float32 scaledRadius(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE || collider->type == ColliderType::SPHERE);

    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    float32 scaleValue = std::max(std::max(xfm->scale().x, xfm->scale().y), xfm->scale().z);

    return scaleValue * collider->radius;
}

Vec3 scaledRect3Lengths(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    return hadamard(xfm->scale(), collider->rect3Lengths);
}

float32 scaledXLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    return xfm->scale().x * collider->rect3Lengths.x;
}

float32 scaledYLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    return xfm->scale().y * collider->rect3Lengths.y;
}

float32 scaledZLength(ColliderComponent* collider)
{
    assert(collider->type == ColliderType::RECT3);
    
    TransformComponent *xfm = getComponent<TransformComponent>(collider->entity);
    return xfm->scale().z * collider->rect3Lengths.z;
}

bool pointInsideCollider(ColliderComponent* collider, Vec3 point)
{
    Quaternion orientation = getComponent<TransformComponent>(collider->entity)->orientation();
    
    Vec3 center = collider->center();

    // Optimize: this calculation doesn't need to be done for the SPHERE case
    Vec3 localX = orientation * Vec3(Axis3D::X);
    Vec3 localY = orientation * Vec3(Axis3D::Y);
    Vec3 localZ = orientation * Vec3(Axis3D::Z);

    switch(collider->type)
    {
        case ColliderType::RECT3:
        {
            float32 xLen = scaledXLength(collider);
            float32 yLen = scaledYLength(collider);
            float32 zLen = scaledZLength(collider);

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

        case ColliderType::SPHERE:
        {
            Vec3 deltaPos = center - point;
            
            float32 radius = scaledRadius(collider);
            float32 radiusSquared = radius * radius;
            
            return (lengthSquared(deltaPos) < radiusSquared);
        } break;

        case ColliderType::CYLINDER:
        {
        } break;

        case ColliderType::CAPSULE:
        {
        } break;

        default:
        {
            assert(false);
        }
    }

    return false;
}
