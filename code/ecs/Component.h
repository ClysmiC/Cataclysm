#pragma once

#include "als/als_types.h"
#include "Entity.h"

struct Component
{
    Component() = default;
    Component(Entity entity);
    
    Entity entity;
};
