#include "TransformComponent.h"
#include "Ecs.h"

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

Vec3 TransformComponent::localPosition()
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);

    if (parentXfm)
    {
        Vec3 result = this->position - parentXfm->position;
        return result;
    }

    return this->position;
}

Quaternion TransformComponent::localOrientation()
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);

    if (parentXfm)
    {
        Quaternion result = relativeRotation(parentXfm->orientation, this->orientation);
        return result;
    }

    return this->orientation;
}

Vec3 TransformComponent::localScale()
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);

    if (parentXfm)
    {
        Vec3 result = hadamardDivide(this->scale, parentXfm->scale);
        return result;
    }

    return this->scale;
}

void TransformComponent::setLocalPosition(Vec3 localPosition)
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);

    if (parentXfm)
    {
        this->position = parentXfm->position + localPosition;
    }
    else
    {
        this->position = localPosition;
    }
}

void TransformComponent::setLocalOrientation(Quaternion localOrientation)
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);

    if (parentXfm)
    {
        this->orientation = localOrientation * parentXfm->orientation;
    }
    else
    {
        this->orientation = localOrientation;
    }
}

void TransformComponent::setLocalScale(Vec3 localScale)
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);

    if (parentXfm)
    {
        this->scale = hadamard(localScale, parentXfm->scale);
    }
    else
    {
        this->scale = localScale;
    }
}

void TransformComponent::resetLocal()
{
    // @Slow: could inline these and combine the "if" checks in eeach
    this->setLocalPosition(Vec3(0));
    this->setLocalOrientation(Quaternion());
    this->setLocalScale(Vec3(1));
}

bool TransformComponent::hasParentTransform()
{
    TransformComponent* parentXfm = getTransformComponent(this->parent);
    return parentXfm != nullptr;
}
