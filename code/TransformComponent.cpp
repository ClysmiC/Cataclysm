#include "TransformComponent.h"

TransformComponent::TransformComponent()
    : TransformComponent(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position)
    : TransformComponent(position, Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation)
    : TransformComponent(position, orientation, Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale)
    : Transform(position, orientation, scale)
{
}

