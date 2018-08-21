#pragma once

#define ALS_TEMP_ALLOC_MAX_CURSORS 256

struct als_temp_alloc_state
{
    bool isInitialized = false;
    char* memory = nullptr;
    char* cursors[ALS_TEMP_ALLOC_MAX_CURSORS];
    int cursorIndex = 0;

    char* firstInvalidByte = nullptr;
    char* highWaterMark = nullptr;
};

void initializeTempAlloc(size_t memoryPoolSize);
void uninitializeTempAlloc();

void pushTempAllocFrame();
void popTempAllocFrame();

void* tempAlloc(size_t bytes);
void* tempAllocArray(size_t bytesPerItem, size_t numItems);
void clearTempAlloc();
