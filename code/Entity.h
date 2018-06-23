#pragma once

#include "als_math.h"
#include <string>

struct TransformComponent;
struct CameraComponent;
struct Ecs;

// TODO: wrap ecs.get<x>Component in functions so callers dont have to reach through pointer

struct Entity
{
    uint32 id;
    Ecs* ecs;
    std::string friendlyName;
};

