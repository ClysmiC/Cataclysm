#pragma once

#include "als_math.h"
#include "Ecs.h"

// Entity is just a unique handle that can be used to retrieve all of its associated components.
// TODO: wrap ecs.get<x>Component in functions so callers dont have to reach through pointer

struct Entity
{
	uint32 id;
    Ecs* ecs;
};

