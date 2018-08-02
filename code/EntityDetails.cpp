#include "EntityDetails.h"

#include "Ecs.h"
#include <algorithm>

void setParent(Entity child, Entity parent)
{
    assert(child.id != 0 && parent.id != 0);
    
    Entity oldParent = getParent(child);

    TransformComponent* childXfm = getTransformComponent(child);
    Vec3 oldWorldPosition;
    Vec3 oldWorldScale;
    Quaternion oldWorldOrientation;

    if (childXfm)
    {
        oldWorldPosition = childXfm->position();
        oldWorldScale = childXfm->scale();
        oldWorldOrientation = childXfm->orientation();
    }
    
    if (oldParent.id != 0)
    {
        std::vector<Entity>* oldParentChildren = getChildren(oldParent);

        auto found = std::find_if(oldParentChildren->begin(), oldParentChildren->end(), [child](Entity e) { return child.id == e.id; });
        if (found != oldParentChildren->end())
        {
            oldParentChildren->erase(found);
        }
    }
    
    EntityDetails* childDetails = getEntityDetails(child);
    childDetails->parent = parent;

    EntityDetails* parentDetails = getEntityDetails(parent);
    parentDetails->children.push_back(child);

    if (childXfm)
    {
        childXfm->setPosition(oldWorldPosition);
        childXfm->setScale(oldWorldScale);
        childXfm->setOrientation(oldWorldOrientation);
    }
}

Entity getParent(Entity e)
{
    Entity result;
    result = getEntityDetails(e)->parent;
    return result;
}

std::vector<Entity>* getChildren(Entity e)
{
    std::vector<Entity>* result;
    result = &getEntityDetails(e)->children;
    return result;
}

string16* getFriendlyName(Entity e)
{
    string16* result;
    result = &getEntityDetails(e)->friendlyName;
    return result;
}

