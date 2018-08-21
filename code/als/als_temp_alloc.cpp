#include "als_temp_alloc.h"
#include "assert.h"
#include <cstdlib>

static als_temp_alloc_state tempAllocState;

void pushTempAllocFrame()
{
    auto& t = tempAllocState;
    t.cursorIndex++;
    assert(t.cursorIndex < ALS_TEMP_ALLOC_MAX_CURSORS);

    t.cursors[t.cursorIndex] = t.cursors[t.cursorIndex - 1];
}

void popTempAllocFrame()
{
    auto& t = tempAllocState;
    t.cursorIndex--;

    assert(t.cursorIndex >= 0);
}

void initializeTempAlloc(size_t memoryPoolSize)
{
    auto& t = tempAllocState;

    assert(!t.isInitialized);

    t.isInitialized = true;
    t.memory = (char*)malloc(memoryPoolSize);
    t.cursorIndex = 0;
    t.cursors[0] = t.memory;
    t.firstInvalidByte = t.memory + memoryPoolSize;
}

void uninitializeTempAlloc()
{
    auto& t = tempAllocState;

    assert(t.isInitialized);

    free(t.memory);

    t.isInitialized = false;
}

void* tempAlloc(size_t bytes)
{
    void* result = tempAllocState.cursors[tempAllocState.cursorIndex];

    tempAllocState.cursors[tempAllocState.cursorIndex] += bytes;

    return result;
}

void* tempAllocArray(size_t bytesPerItem, size_t numItems)
{
    return tempAlloc(bytesPerItem * numItems);
}

void clearTempAlloc()
{
    auto& t = tempAllocState;

    if (t.cursors[t.cursorIndex] > t.highWaterMark)
    {
        t.highWaterMark = t.cursors[t.cursorIndex];
    }

    if (t.cursors[t.cursorIndex] > t.firstInvalidByte)
    {
        // @Think: Do we want to add this check to every tempAlloc and return null if we have overflow?
        //         Probably not... because realistically we won't check for failure every time we call tempAlloc.
        //         The best strategy is probably just to make the memory pool large enough that we never have to worry!

        assert(false); // Exceeded temporary allocator capacity!
    }

    t.cursors[0] = t.memory;
    t.cursorIndex = 0;
}
