#pragma once

#include "Entity.h"

template <class T>
struct ComponentGroup
{
    ComponentGroup();

    Entity entity;

    T* components;
    uint32 numComponents;
};

template<class T>
ComponentGroup<T>::ComponentGroup()
{
    entity.id = 0;
    components = nullptr;
    numComponents = 0;
}

