#pragma once

#include "Entity.h"
#include "als_bucket_array.h"

template <class T, uint32 BUCKET_SIZE>
struct ComponentGroup
{
    ComponentGroup()
    {
        entity.id = 0;
        numComponents = 0;
        bucketArray = nullptr;
    }

    T& operator [] (uint32 index)
    {
        return *(bucketArray->addressOf(this->components[index]));
    }

    PotentiallyStaleEntity entity;

    BucketArray<T, BUCKET_SIZE>* bucketArray;
    BucketLocator components[MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY];
    uint32 numComponents = 0;
};

