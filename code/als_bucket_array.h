#pragma once

#include "Types.h"
#include <vector>

//
// Implementation guided by the following video:
// Demo: Iterators, (Overloading x Polymorphism) - Jon Blow
// https://www.youtube.com/watch?v=COQKyOCAxOQ&index=24&list=PLmV5I2fxaiCKfxMBrNsU1kgKJXD3PkyxO
//

#define FOR_BUCKET_ARRAY(bucketArray) \
    for ( auto it = (bucketArray)._iteratorStart(); (it = (bucketArray).iterate(it)).flag != _BucketArrayIteratorFlag::END ; )

enum class _BucketArrayIteratorFlag { VALID, START, END };

struct BucketLocator
{
    int32 bucketIndex;
    int32 slotIndex;

    BucketLocator() = default;
    BucketLocator(int32 bucketIndex, int32 slotIndex)
    {
        this->bucketIndex = bucketIndex;
        this->slotIndex = slotIndex;
    }
};

template<typename T, uint32 BUCKET_SIZE = 16>
struct BucketArray
{
    struct Bucket
    {
        // uint32 index;
        uint32 count = 0;
        bool occupied[BUCKET_SIZE] = { 0 };
        T data[BUCKET_SIZE];
    };

    struct BucketIterator
    {
        uint32 index = 0;
        BucketLocator locator = BucketLocator(0, 0);
        T* ptr;
        _BucketArrayIteratorFlag flag = BAIF_VALID;
    };


    std::vector<Bucket* > buckets;
    std::vector<Bucket* > unfullBuckets;

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
    BucketLocator occupyEmptySlot()
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

    T* addressOf(BucketLocator locator)
    {
        return &(this->buckets[locator.bucketIndex].data[locator.slotIndex]);
    }

    T* addressOf(BucketLocator locator, uint32 index)
    {
        const uint32 indexCopyForLookingAtInDebugger = index;
        
        int32 bucketIndex = locator.index;
        int32 slotIndex = locator.slotIndex;
        
        index -= min(index, BUCKET_SIZE - slotIndex);
        while (index > 0)
        {
            bucketIndex++;
            slotIndex = 0;
        }

        
        T* result = addressOf(BucketLocator(bucketIndex, slotIndex));
        return result;
    }
    
    BucketLocator add(T item)
    {
        BucketLocator result = this->occupyEmptySlot();
        T* slot = this->addressOf(locator);
        *slot = item;

        return result;
    }

    bool isOccupied(BucketLocator locator)
    {
        if (locator.bucketIndex < 0 || locator.slotIndex < 0) return false;
        
        return this->buckets[locator.bucketIndex].occupied[locator.slotIndex];
    }

    bool isEmpty()
    {
        return this->count == 0;
    }
    
    BucketLocator _firstLocation()
    {
        if (this->count == 0) return BucketLocator(-1, -1);
        
        BucketLocator locator(0, 0);

        if (this->isOccupied(locator)) return locator;

        locator = this->next(locator);
        return locator;
    }

    BucketIterator _iteratorStart()
    {
        BucketIterator result;
        result.flag = _BucketArrayIteratorFlag::START;
        return result;
    }

    bool hasNext(BucketLocator locator)
    {
        int32 bucketIndex = locator.bucketIndex;
        int32 slotIndex = locator.slotIndex;

        if (bucketIndex * BUCKET_SIZE + slotIndex < this->count - 1) return true;

        while(bucketIndex < this->buckets.size())
        {
            slotIndex++;

            if (slotIndex == BUCKET_SIZE)
            {
                slotIndex = 0;
                bucketIndex++;
            }

            if (this->buckets[bucketIndex].occupied[slotIndex]) return true;
        }

        return false;
    }

    BucketLocator next(BucketLocator locator)
    {
        int32 bucketIndex = locator.bucketIndex;
        int32 slotIndex = locator.slotIndex;

        while(bucketIndex < this->buckets.size())
        {
            slotIndex++;

            if (slotIndex == BUCKET_SIZE)
            {
                slotIndex = 0;
                bucketIndex++;
            }

            if (this->buckets[bucketIndex].occupied[slotIndex])
            {
                return BucketLocator(bucketIndex, slotIndex);
            }
        }

        return BucketLocator(-1, -1);
    }

    BucketIterator iterate(BucketIterator it)
    {
        BucketIterator result;

        if (it.flag == _BucketArrayIteratorFlag::START)
        {
            result.locator = this->_firstLocation();
            result.index = 0;
        }
        else
        {
            result.locator = this->next(it.locator);
            result.index = it.index++;
        }

        if (!this->isOccupied(result.locator))
        {
            result.flag = _BucketArrayIteratorFlag::END;
        }

        return result;
    }

    ~BucketArray()
    {
        for (auto it : buckets)
        {
            delete it;
        }
    }
};
