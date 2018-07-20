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
{
    // @Think: should thisk be local or world?
    //         If they can't set a parent at construction, then they
    //         are equivalent. But if I allow them to, I need to think
    //         about this.
    this->setLocalPosition(position);
    this->setLocalOrientation(orientation);
    this->setLocalScale(scale);
}

void
TransformComponent::setLocalPosition(Vec3 position)
{
    _localTransform.setPosition(position);
    markSelfAndChildrenDirty();
}
    
Vec3
TransformComponent::localPosition()
{
    return _localTransform.position();
}

void
TransformComponent::setLocalOrientation(Quaternion orientation)
{
    _localTransform.setOrientation(orientation);
    markSelfAndChildrenDirty();
}
    
Quaternion
TransformComponent::localOrientation()
{
    return _localTransform.orientation();
}

void
TransformComponent::setLocalScale(Vec3 scale)
{
    _localTransform.setScale(scale);
    markSelfAndChildrenDirty();
}
    
Vec3
TransformComponent::localScale()
{
    return _localTransform.scale();
}

void
TransformComponent::setWorldPosition(Vec3 position)
{
    TransformComponent* p = getTransformComponent(parent);
    if (p)
    {
        Mat4 worldToParent = inverse(p->worldTransform()->matrix());
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

Transform*
TransformComponent::worldTransform()
{
    if (cachedWorldDirty) recalculateWorld();
    return &cachedWorldTransform;
}

Transform*
TransformComponent::localTransform()
{
    return &_localTransform;
}

void
TransformComponent::recalculateWorld()
{
    TransformComponent* p = getTransformComponent(parent);

    if (p)
    {
        Transform* parentWorldXfm = p->worldTransform();
        cachedWorldTransform = multiplyTransforms(_localTransform, *parentWorldXfm);
    }
    else
    {
        cachedWorldTransform = _localTransform;
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

void
TransformComponent::setLocalTransform(Transform transform)
{
    setLocalPosition(transform.position());
    setLocalOrientation(transform.orientation());
    setLocalScale(transform.scale());
}

void
TransformComponent::setWorldTransform(Transform transform)
{
    setWorldPosition(transform.position());
    setWorldOrientation(transform.orientation());
    setWorldScale(transform.scale());
}
