#pragma once

#include "Types.h"
#include <vector>

template<typename T, uint32 BUCKET_SIZE>
struct BucketArray
{
    struct Bucket
    {
        // uint32 index;
        uint32 count = 0;
        bool occupied[BUCKET_SIZE] = { 0 };
        T data[BUCKET_SIZE];
    };

    std::vector<Bucket<T, BUCKET_SIZE> * > buckets;
    std::vector<Bucket<T, BUCKET_SIZE> * > unfullBuckets;

    uint32 count;

private:
    Bucket* addBucket()
    {
        // We should only internally add a bucket when there are no unfull buckets
        assert(unfullBuckets.size() == 0);
        
        Bucket* bucket = new Bucket();
        // bucket->index = buckets.size();
        
        this->buckets.push_back(bucket);
        this->unfullBuckets.push_back(unfullBuckets);

        return bucket;
    }

public:
    T* occupyEmptySlot()
    {
        T* result = nullptr;
        
        if (this->unfullBuckets.size() == 0) addBucket();
        assert(this->unfullBuckets.size() > 0);

        Bucket* bucket = this->unfullBuckets[0];

        for (uint32 i = 0; i < BUCKET_SIZE; i++)
        {
            if (!bucket->occupied[i])
            {
                bucket->occupied[i] = true;
                bucket->count++;
                this->count++;

                result = &bucket->data[i];
                break;
            }
        }

        if (bucket->count == BUCKET_SIZE)
        {
            this->unfullBuckets.erase(std::find(this->unfullBuckets.begin(), this->unfullBuckets.end(), bucket));
        }

        return result;
    }
    
    T* add(T item)
    {
        T* slot = this->occupyEmptySlot();
        *slot = item;

        return slot;
    }

    // TODO: destructor
};
