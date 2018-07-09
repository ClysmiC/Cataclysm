#pragma once

#include "Types.h"
#include "Entity.h"

struct Component
{
    Component() = default;
    Component(Entity entity);
    
    Entity entity;
};
