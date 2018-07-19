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

void
TransformComponent::setLocalPosition(Vec3 position)
{
    localTransform.setPosition(position);
    markSelfAndChildrenDirty();
}
    
Vec3
TransformComponent::localPosition()
{
    return localTransform.position();
}

void
TransformComponent::setLocalOrientation(Quaternion orientation)
{
    localTransform.setOrientation(orientation);
    markSelfAndChildrenDirty();
}
    
Quaternion
TransformComponent::localOrientation()
{
    return localTransform.orientation();
}

void
TransformComponent::setLocalScale(Vec3 scale)
{
    localTransform.setScale(scale);
    markSelfAndChildrenDirty();
}
    
Vec3
TransformComponent::localScale()
{
    return localTransform.scale();
}

void
TransformComponent::setWorldPosition(Vec3 position)
{
    TransformComponent* p = getTransformComponent(parent);
    if (p)
    {
        Mat4 worldToParent = inverse(p->worldTransform().matrix());
        Vec4 newLocal = worldToParent * Vec4(position, 1.0);
        this->setLocalPosition(newLocal.xyz());
    }
    else
    {
        this->setLocalPosition(position);
    }
}

Vec3
TransformComponent::worldPosition()
{
    if (cachedWorldDirty) recalculateWorld();
    return cachedWorldTransform.position();
}

void
TransformComponent::setWorldOrientation(Quaternion orientation)
{
    TransformComponent* p = getTransformComponent(parent);
    if (p)
    {
        Quaternion rotationNeeded = relativeRotation(p->worldOrientation(), orientation);
        this->setLocalOrientation(rotationNeeded);
    }
    else
    {
        this->setLocalOrientation(orientation);
    }
}

Quaternion
TransformComponent::worldOrientation()
{
    if (cachedWorldDirty) recalculateWorld();
    return cachedWorldTransform.orientation();
}

void
TransformComponent::setWorldScale(Vec3 scale)
{
    TransformComponent* p = getTransformComponent(parent);
    if (p)
    {
        this->setLocalScale(hadamardDivide(scale, p->worldScale()));
    }
    else
    {
        this->setLocalScale(scale);
    }
}

Vec3
TransformComponent::worldScale()
{
    if (cachedWorldDirty) recalculateWorld();
    return cachedWorldTransform.scale();
}

Transform
TransformComponent::worldTransform()
{
    if (cachedWorldDirty) recalculateWorld();
    return cachedWorldTransform;
}

void
TransformComponent::recalculateWorld()
{
    TransformComponent* p = getTransformComponent(parent);

    if (p)
    {
        Transform parentWorldXfm = p->worldTransform();
        cachedWorldTransform = multiplyTransforms(localTransform, parentWorldXfm);
    }
    else
    {
        cachedWorldTransform = localTransform;
    }

    // TODO: implement multiply
    cachedWorldDirty = false;
}

void
TransformComponent::markSelfAndChildrenDirty()
{
    cachedWorldDirty = true;
    for (auto t : children)
    {
        t->markSelfAndChildrenDirty();
    }
}
