#pragma once

#include "ecs/Entity.h"
#include "ecs/IComponent.h"
#include "als/als_fixed_string.h"
#include <vector>

enum EntityFlags : uint64
{
    EntityFlag_None              = 0,
        
    EntityFlag_Static            = 1 << 0,
    EntityFlag_MarkedForDeletion = 1 << 1,
};

extern EntityFlags g_defaultEntityFlags;

struct EntityDetails : public IComponent
{
    PotentiallyStaleEntity parent;
    uint64 flags = 0;
    string16 friendlyName = "";
    std::vector<PotentiallyStaleEntity> children;

    static const bool multipleAllowedPerEntity = false;
};

void removeParent(Entity e);
void setParent(Entity child, Entity parent);
PotentiallyStaleEntity getParent(Entity e);
std::vector<PotentiallyStaleEntity>* getChildren(Entity e);
string16* getFriendlyName(Entity e);
string32 getFriendlyNameAndId(Entity e);
