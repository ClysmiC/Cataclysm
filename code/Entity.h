#pragma once

#include "als_math.h"
#include "als_fixed_string.h"

struct TransformComponent;
struct CameraComponent;
struct Ecs;

struct Entity
{
    uint32 id;
    uint64 flags;
    Ecs* ecs;
    string16 friendlyName;
};

