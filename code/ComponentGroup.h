#pragma once

#include "Entity.h"
#include "als_bucket_array.h"

#define FOR_COMPONENT_GROUP(cgroup)             \
    for ( auto it = (cgroup).bucketArray->_initIterator((cgroup).firstComponent, (cgroup).numComponents); (it = (cgroup).bucketArray->iterate(it)).flag != _BucketArrayIteratorFlag::POST_ITERATE ; )


template <class T, uint32 BUCKET_SIZE>
struct ComponentGroup
{
    ComponentGroup()
    {
        entity.id = 0;
        numComponents = 0;
        bucketArray = nullptr;
    }

    // @Slow: linearly searches forward from firstComponent. To quickly loop, use FOR_COMPONENT_GROUP macro
    T& operator [] (uint32 index)
    {
        return *(bucketArray->addressOf(firstComponent, index));
    }

    Entity entity;

    BucketArray<T, BUCKET_SIZE>* bucketArray;
    BucketLocator firstComponent;
    uint32 numComponents;
};

