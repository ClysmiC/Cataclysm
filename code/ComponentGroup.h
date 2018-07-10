#pragma once

#include "Entity.h"
#include "als_bucket_array.h"

template <class T>
struct ComponentGroup
{
    ComponentGroup();

    Entity entity;

    BucketLocator firstComponent;
    uint32 numComponents;
};

template<class T>
ComponentGroup<T>::ComponentGroup()
{
    entity.id = 0;
}

