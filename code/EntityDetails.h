#pragma once

#include "Entity.h"
#include "Component.h"
#include "als_fixed_string.h"
#include <vector>

struct EntityDetails : public Component
{
    Entity parent;
    uint64 flags;
    string16 friendlyName;
    std::vector<Entity> children;
};

void removeParent(Entity e);
void setParent(Entity child, Entity parent);
Entity getParent(Entity e);
std::vector<Entity>* getChildren(Entity e);
string16* getFriendlyName(Entity e);
