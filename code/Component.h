#pragma once

#include "Entity.h"

struct Component
{
    Component() = default;
    Component(Entity entity);
    
    Entity entity;
};
