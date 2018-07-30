#include "TransformComponent.h"
#include "Ecs.h"
#include <algorithm>

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

TransformComponent::TransformComponent(ITransform& transform)
    : TransformComponent(transform.position(), transform.orientation(), transform.scale())
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale)
    : ITransform(position, orientation, scale)
{
}

ITransform* TransformComponent::getParent()
{
    Entity parent = ::getParent(this->entity);
    TransformComponent* result = getTransformComponent(parent);
    return result;
}
     
std::vector<ITransform*> TransformComponent::getChildren()
{
    std::vector<ITransform*> result;

    
    for (auto e : *::getChildren(this->entity))
    {
        TransformComponent* xfm = getTransformComponent(e);

        if (xfm)
        {
            result.push_back(xfm);
        }
    }
    
    return result;
}
