#include "EntityDetails.h"

#include "Game.h"
#include "ecs/Ecs.h"
#include <algorithm>

#include "ecs/components/TransformComponent.h"

EntityFlags g_defaultEntityFlags = EntityFlag_Static;

void removeParent(Entity e)
{
    if (e.id == 0) return;

    PotentiallyStaleEntity staleParent = getParent(e);
    Entity parent = getEntity(getGame(e), &staleParent);

    if (parent.id == 0) return;

    TransformComponent* xfm = getComponent<TransformComponent>(e);
    Vec3 oldWorldPosition;
    Vec3 oldWorldScale;
    Quaternion oldWorldOrientation;

    //
    // Store old world xfm
    //
    if (xfm)
    {
        oldWorldPosition = xfm->position();
        oldWorldScale = xfm->scale();
        oldWorldOrientation = xfm->orientation();
    }

    //
    // Remove self from parent's children
    //
    std::vector<PotentiallyStaleEntity>* parentChildren = getChildren(parent);
    auto found = std::find_if(parentChildren->begin(), parentChildren->end(), [e](PotentiallyStaleEntity e2) { return e.id == e2.id; });
    if (found != parentChildren->end())
    {
        parentChildren->erase(found);
    }
    else
    {
        assert(false);
    }

    //
    // Set parent to 0
    //
    EntityDetails* details = getComponent<EntityDetails>(e);
    details->parent.id = 0;

    //
    // Re-apply old world xfm
    //
    if (xfm)
    {
        xfm->setPosition(oldWorldPosition);
        xfm->setScale(oldWorldScale);
        xfm->setOrientation(oldWorldOrientation);
    }
}

void setParent(Entity child, Entity parent)
{
    if (child.id == 0) return;

    //
    // Remove old parent
    //
    removeParent(child);

    if (parent.id == 0) return;

    //
    // Store old world xfm
    //
    TransformComponent* childXfm = getComponent<TransformComponent>(child);
    Vec3 oldWorldPosition;
    Vec3 oldWorldScale;
    Quaternion oldWorldOrientation;
    if (childXfm)
    {
        oldWorldPosition = childXfm->position();
        oldWorldScale = childXfm->scale();
        oldWorldOrientation = childXfm->orientation();
    }

    //
    // Set parent
    //
    EntityDetails* childDetails = getComponent<EntityDetails>(child);
    childDetails->parent = PotentiallyStaleEntity(parent);

    EntityDetails* parentDetails = getComponent<EntityDetails>(parent);
    parentDetails->children.push_back(PotentiallyStaleEntity(child));

    if (childXfm)
    {
        childXfm->setPosition(oldWorldPosition);
        childXfm->setScale(oldWorldScale);
        childXfm->setOrientation(oldWorldOrientation);
    }
}

PotentiallyStaleEntity getParent(Entity e)
{
    PotentiallyStaleEntity result;
    result = getComponent<EntityDetails>(e)->parent;
    return result;
}

std::vector<PotentiallyStaleEntity>* getChildren(Entity e)
{
    std::vector<PotentiallyStaleEntity>* result;
    result = &getComponent<EntityDetails>(e)->children;
    return result;
}

string16* getFriendlyName(Entity e)
{
    string16* result;
    result = &getComponent<EntityDetails>(e)->friendlyName;
    return result;
}

string32 getFriendlyNameAndId(Entity e)
{
    EntityDetails* details = getComponent<EntityDetails>(e);
    string32 result;

    if (e.id == 0)
    {
        result = "null (0)";
    }
    else
    {
        sprintf_s((char*)result.cstr(), 32, "%s (%u)", details->friendlyName.cstr(), e.id);
        result.invalidateLength();
    }
    
    return result;
}

