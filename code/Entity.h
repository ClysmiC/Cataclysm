#pragma once

#include "als_math.h"
#include "als_fixed_string.h"

struct TransformComponent;
struct CameraComponent;
struct Ecs;

#define MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY 16

struct Entity
{
    uint32 id = 0;
    Ecs* ecs = nullptr;
};

