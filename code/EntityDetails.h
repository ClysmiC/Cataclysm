#pragma once

#include "Entity.h"
#include "Component.h"
#include "als_fixed_string.h"
#include <vector>

enum EntityFlags : uint64
{
    EntityFlag_Static            = 1 << 0,
    EntityFlag_MarkedForDeletion = 1 << 1,
};

struct EntityDetails : public Component
{
    PotentiallyStaleEntity parent;
    uint64 flags = 0;
    string16 friendlyName = "";
    std::vector<PotentiallyStaleEntity> children; // @TODO: these should probably just be ID's

    static const bool multipleAllowedPerEntity = false;
};

void removeParent(Entity e);
void setParent(Entity child, Entity parent);
PotentiallyStaleEntity getParent(Entity e);
std::vector<PotentiallyStaleEntity>* getChildren(Entity e);
string16* getFriendlyName(Entity e);
string32 getFriendlyNameAndId(Entity e);
