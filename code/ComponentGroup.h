#pragma once

#include "Entity.h"

template <class T>
struct ComponentGroup
{
    Entity entity;

    T* components;
    uint32 numComponents;
};
