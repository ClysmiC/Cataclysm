#pragma once

#include "als/als_types.h"
#include "Entity.h"

struct IComponent
{
    IComponent() = default;
    IComponent(Entity entity);
    
    Entity entity; // Note: this needs to be updated for all of an entity's components if the entity moves ECSs

    virtual void onAddComponent();
    virtual void onRemoveComponent();
};
