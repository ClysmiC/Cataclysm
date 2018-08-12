#pragma once

#include "als_types.h"
#include <vector>
#include <assert.h>
#include "als_util.h"

//
// Implementation guided by the following video:
// Demo: Iterators, (Overloading x Polymorphism) - Jon Blow
// https://www.youtube.com/watch?v=COQKyOCAxOQ&index=24&list=PLmV5I2fxaiCKfxMBrNsU1kgKJXD3PkyxO
//

#define FOR_BUCKET_ARRAY(bucketArray) \
    for ( auto it = (bucketArray)._initIterator(); (it = (bucketArray).iterate(it)).flag != _BucketArrayIteratorFlag::POST_ITERATE ; )

enum class _BucketArrayIteratorFlag { ITERATING, PRE_ITERATE, POST_ITERATE };

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
        uint32 index = 0;
        uint32 count = 0;
        bool occupied[BUCKET_SIZE] = { 0 };
        T data[BUCKET_SIZE];
    };

    struct BucketIterator
    {
        uint32 index = 0;
        uint32 countToIterate = 0;
        BucketLocator locator = BucketLocator(0, 0);
        T* ptr = nullptr;
        _BucketArrayIteratorFlag flag = _BucketArrayIteratorFlag::ITERATING;
    };


    std::vector<Bucket* > buckets;
    std::vector<Bucket* > unfullBuckets;

    uint32 count = 0;

private:
    Bucket* addBucket()
    {
        // We should only internally add a bucket when there are no unfull buckets
        assert(unfullBuckets.size() == 0);
        
        Bucket* bucket = new Bucket();
        bucket->index = buckets.size();
        
        this->buckets.push_back(bucket);
        this->unfullBuckets.push_back(bucket);

        return bucket;
    }


public:
    BucketLocator occupyEmptySlot()
    {
        BucketLocator result;
        
        if (this->unfullBuckets.size() == 0) addBucket();
        assert(this->unfullBuckets.size() > 0);

        Bucket* bucket = this->unfullBuckets[0];
        result.bucketIndex = bucket->index;

        for (uint32 i = 0; i < BUCKET_SIZE; i++)
        {
            if (!bucket->occupied[i])
            {
                bucket->occupied[i] = true;
                bucket->count++;
                this->count++;
                result.slotIndex = i;

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
        return &(this->buckets[locator.bucketIndex]->data[locator.slotIndex]);
    }

    T* addressOf(BucketLocator locator, uint32 index)
    {
        const uint32 indexCopyForLookingAtInDebugger = index;
        
        int32 bucketIndex = locator.bucketIndex;
        int32 slotIndex = locator.slotIndex;
        
        uint32 advanceSlotIndexAmount = min(index, BUCKET_SIZE - slotIndex);
        slotIndex += advanceSlotIndexAmount;

        index -= advanceSlotIndexAmount;
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
        T* slot = this->addressOf(result);
        *slot = item;

        return result;
    }

    bool remove(BucketLocator locator)
    {
        if (this->isOccupied(locator))
        {
            Bucket* bucket = this->buckets[locator.bucketIndex];
            bucket->occupied[locator.slotIndex] = false;
            bucket->count--;
            this->count--;

            if (bucket->count == BUCKET_SIZE - 1)
            {
                this->unfullBuckets.push_back(bucket);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    bool isOccupied(BucketLocator locator)
    {
        if (locator.bucketIndex < 0 || locator.slotIndex < 0 || this->buckets.empty()) return false;
        
        return this->buckets[locator.bucketIndex]->occupied[locator.slotIndex];
    }

    bool isEmpty()
    {
        return this->count == 0;
    }

    BucketIterator _initIterator(BucketLocator startingLocator = BucketLocator(0, 0), uint32 countToIterate = -1)
    {
        BucketIterator result;
        result.flag = _BucketArrayIteratorFlag::PRE_ITERATE;
        result.locator = startingLocator;
        result.countToIterate = countToIterate;
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

            if (this->buckets[bucketIndex]->occupied[slotIndex]) return true;
        }

        return false;
    }

    BucketLocator next(BucketLocator locator)
    {
        uint32 bucketIndex = locator.bucketIndex;
        uint32 slotIndex = locator.slotIndex;

        while(bucketIndex < this->buckets.size())
        {
            slotIndex++;

            if (slotIndex == BUCKET_SIZE)
            {
                slotIndex = 0;
                bucketIndex++;

                if (bucketIndex == this->buckets.size()) break;
            }

            if (this->buckets[bucketIndex]->occupied[slotIndex])
            {
                return BucketLocator(bucketIndex, slotIndex);
            }
        }

        return BucketLocator(-1, -1);
    }

    BucketIterator iterate(BucketIterator it)
    {
        BucketIterator result;

        if (it.flag == _BucketArrayIteratorFlag::POST_ITERATE)
        {
            result.flag = _BucketArrayIteratorFlag::POST_ITERATE;
            return result;
        }

        if (it.flag == _BucketArrayIteratorFlag::PRE_ITERATE)
        {
            if (this->isOccupied(it.locator))
            {
                result.locator = it.locator;
            }
            else
            {
                result.locator = this->next(it.locator);
            }

            result.flag = _BucketArrayIteratorFlag::ITERATING;
            result.index = 0;
        }
        else
        {
            result.locator = this->next(it.locator);
            result.index = it.index + 1;
        }

        result.countToIterate = it.countToIterate;

        if (this->isOccupied(result.locator) && (result.index < result.countToIterate || result.countToIterate == (uint32)-1))
        {
            result.ptr = this->addressOf(result.locator);
        }
        else
        {
            // Reached end of bucket array
            result.flag = _BucketArrayIteratorFlag::POST_ITERATE;
        }

        return result;
    }

    T* at(uint32 index)
    {
        if (index >= this->count) return nullptr;

        BucketIterator it = _initIterator();

        for (uint32 i = 0; i <= index; i++)
        {
            it = iterate(it);

            if (it.flag == _BucketArrayIteratorFlag::POST_ITERATE)
            {
                // This should get caught at the initial check this function makes
                assert(false);
                return nullptr;
            }
        }

        return it.ptr;
    }

    ~BucketArray()
    {
        for (auto it : buckets)
        {
            delete it;
        }
    }
};
