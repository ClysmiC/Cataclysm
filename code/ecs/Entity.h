#pragma once

#include "als/als_math.h"

struct TransformComponent;
struct CameraComponent;
struct Ecs;

// Note: this SHOULD be enough even for the ones that can be sprawling, like lots of submeshes (render components) or colliders
#define MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY 64

struct Entity
{
    uint32 id = 0;
    Ecs* ecs = nullptr;

    inline bool operator == (Entity other)
    {
        if (this->id == 0 || other.id == 0) return false;
        return this->id == other.id;
    }
};

// Note: This is an entity that might have an incorrect ECS pointer. Use getEntity(..) to get an entity that
//       is guaranteed to have the correct ECS pointer. If the PotentiallyStaleEntity DOES have a correct ECS pointer
//       (which will usually be the case), turning it into an Entity is an O(1) operation. getEntity(..) will also update
//       the PotentiallyStaleEntity with the correct pointer.
//
//       Anywhere where an Entity is stored in a struct instead of a PotentiallyStaleEntity, that struct is committing to
//       keeping the ECS pointer up to date if the entity is moved
struct PotentiallyStaleEntity
{
    PotentiallyStaleEntity() = default;
    PotentiallyStaleEntity(Entity e);
    
    uint32 id = 0;
    Ecs* ecs = nullptr; // Note: This is only allowed to be null if id is 0. If you set the id, you should set the ecs too! Or just use the implicit constructor and assign from an Entity    
};
