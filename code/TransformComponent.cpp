#include "TransformComponent.h"
#include "Ecs.h"
#include <algorithm>

TransformComponent::TransformComponent(Entity e)
    : TransformComponent(e, Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Entity e, Vec3 position)
    : TransformComponent(e, position, Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Entity e, Vec3 position, Quaternion orientation)
    : TransformComponent(e, position, orientation, Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Entity e, ITransform& transform)
    : TransformComponent(e, transform.position(), transform.orientation(), transform.scale())
{
}

TransformComponent::TransformComponent(Entity e, Vec3 position, Quaternion orientation, Vec3 scale)
    : Component(e)
    , ITransform(position, orientation, scale)
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
