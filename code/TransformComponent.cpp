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
    this->parent.id = 0;
}

ITransform* TransformComponent::getParent()
{
    TransformComponent* result = getTransformComponent(this->parent);
    return result;
}
     
std::vector<ITransform*> TransformComponent::getChildren()
{
    std::vector<ITransform*> result;

    for (auto e : this->children)
    {
        TransformComponent* xfm = getTransformComponent(e);

        if (xfm)
        {
            result.push_back(xfm);
        }
    }
    
    return result;
}

void setParent(TransformComponent* xfm, Entity e)
{
    if (xfm->parent.id != 0)
    {
        TransformComponent* oldParent = getTransformComponent(xfm->parent);

        // @Untested. Dumb C++ idioms...
        auto found = std::find_if(oldParent->children.begin(), oldParent->children.end(), [xfm](Entity e) { return xfm->entity.id == e.id; });
        if (found != oldParent->children.end())
        {
            oldParent->children.erase(found);
        }
    }
    
    xfm->parent = e;

    TransformComponent* parentXfm = getTransformComponent(xfm->parent);

    if (parentXfm)
    {
        parentXfm->children.push_back(xfm->entity);
    }
}
