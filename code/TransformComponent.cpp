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

TransformComponent::TransformComponent(LiteTransform transform)
    : TransformComponent(transform.position, transform.orientation, transform,scale)
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale)
{
    // @Think: should thisk be local or world?
    //         If they can't set a parent at construction, then they
    //         are equivalent. But if I allow them to, I need to think
    //         about this.
    this->parent.id = 0;
    this->setLocalPosition(position);
    this->setLocalOrientation(orientation);
    this->setLocalScale(scale);
}

Transform* getParent()
{
    TransformComponent* result = getTransformComponent(parent);
    return result;
}
     
std::vector<Transform*> getChildren()
{
    std::vector<Transform*> result;

    for (auto entity : this->children)
    {
        TransformComponent* xfm = getTransformComponent(entity);

        if (xfm)
        {
            result.push_back(xfm);
        }
    }
    
    return result;
}
