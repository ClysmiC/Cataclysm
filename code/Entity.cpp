#include "Entity.h"
#include "Ecs.h"

#include "Entity.h"

PotentiallyStaleEntity::PotentiallyStaleEntity(Entity e)
{
    this->id = e.id;
    this->ecs = e.ecs;
}
