#include "TransformComponent.h"
#include "ecs/Ecs.h"
#include "Game.h"
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
    : IComponent(e)
    , ITransform(position, orientation, scale)
{
}

ITransform* TransformComponent::getParent()
{
    PotentiallyStaleEntity parent = ::getParent(this->entity);
    TransformComponent* result = getComponent<TransformComponent>(getEntity(getGame(this->entity), &parent));
    return result;
}
     
std::vector<ITransform*> TransformComponent::getChildren()
{
    std::vector<ITransform*> result;

    
    for (auto e : *::getChildren(this->entity))
    {
        TransformComponent* xfm = getComponent<TransformComponent>(getEntity(getGame(this->entity), &e));

        if (xfm)
        {
            result.push_back(xfm);
        }
    }
    
    return result;
}
