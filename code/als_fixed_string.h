#pragma once

#include "Types.h"

template <uint16 SIZE> struct _als_fixed_string
{
    uint16 length;
    char data[SIZE];
    char _safety_byte; // always set to '\0' so that a pointer to a string of length SIZE will still have a null terminator

    _als_fixed_string()
    {
        length = 0;
        data[0] = '\0';
        _safety_byte = '\0';
    }
    
    _als_fixed_string(const char* cstr)
    {
        length = 0;
        _safety_byte = '\0';
        
        for (int i = 0; i < SIZE; i++)
        {
            char c = cstr[i];
            data[i] = c;
            
            if (c == '\0') break;

            length++;
        }
    }

    char operator[] (int i)
    {
        return this->data[i];
    }
};

constexpr uint16 _myUInt16Max(const uint16 lhs, const uint16 rhs)
{
    return lhs >= rhs ? lhs : rhs;
}

//
// Concatenates two fixed strings.
// The resulting type is the type of the larger of the two fixed string types.
// Any overflow gets truncated.
// Example: string32 + string16 would return a potentially truncated string32
//
template <uint16 LHS_SIZE, uint16 RHS_SIZE>
_als_fixed_string<_myUInt16Max(LHS_SIZE, RHS_SIZE)> operator + (const _als_fixed_string<LHS_SIZE> &lhs, const _als_fixed_string<RHS_SIZE> &rhs)
{
    constexpr uint16 resultSize = _myUInt16Max(LHS_SIZE, RHS_SIZE);
    _als_fixed_string<resultSize> result;

   for (uint16 i = 0; i < lhs.length; i++)
   {
       result.data[i] = lhs.data[i];
   }

   for (uint16 i = 0; i < rhs.length; i++)
   {
       uint16 index = lhs.length + i;

       if (index >= resultSize) break;
       
       result.data[index] = rhs.data[i];
   }

   result.length = lhs.length + rhs.length;

   if (result.length > resultSize)
   {
       result.length = resultSize;
   }
   else
   {
       // NOTE: if result.length = SIZE, this is writing \0 into the safety byte
       // (which is fine... just be aware of it)
       result.data[result.length] = '\0';
   }

   return result;
}

//
// Appends a fixed string to an existing fixed string. If the modified string overflows, truncate it.
//
template <uint16 LHS_SIZE, uint16 RHS_SIZE>
_als_fixed_string<LHS_SIZE>& operator += (_als_fixed_string<RHS_SIZE> &lhs, const _als_fixed_string<RHS_SIZE> &rhs)
{
    for (uint16 i = 0; i < rhs.length; i++)
    {
        uint16 index = lhs.length + i;

        if (index >= SIZE) break;
        
        lhs.data[index] = rhs.data[i];
    }

    lhs.length += rhs.length;

    if (lhs.length > SIZE)
    {
        lhs.length = SIZE;
    }
    else
    {
        // NOTE: if lhs.length = SIZE, this is writing \0 into the safety byte
        // (which is fine... just be aware of it)
        lhs.data[lhs.length] = '\0';
    }

    return lhs;
}

//
// Appends a c-string to an existing fixed string. If the modified string overflows, truncate it.
//
template<uint16 SIZE>
_als_fixed_string<SIZE>& operator += (_als_fixed_string<SIZE> &lhs, const char* rhs)
{
    for (uint16 lhsIndex = lhs.length, rhsIndex = 0; lhsIndex < SIZE; lhsIndex++, rhsIndex++)
    {
        char c = rhs[rhsIndex];
        lhs.data[lhsIndex] = c;

        if (c == '\0') break;
        
        lhs.length++;
    }

    if (lhs.length == SIZE)
    {
        // No need to write '\0' since we have the safety byte
    }

    return lhs;
}

template<uint16 SIZE>
_als_fixed_string<SIZE> operator + (const char* lhs, const _als_fixed_string<SIZE> &rhs)
{
    _als_fixed_string<SIZE> result = lhs;
    result += rhs;
    return result;
}

template<uint16 SIZE>
_als_fixed_string<SIZE> operator + (const _als_fixed_string<SIZE> &lhs, const char* rhs)
{
    _als_fixed_string<SIZE> result = lhs;
    result += rhs;
    return result;
}

typedef _als_fixed_string<8> string8;
typedef _als_fixed_string<16> string16;
typedef _als_fixed_string<32> string32;
typedef _als_fixed_string<64> string64;
typedef _als_fixed_string<128> string128;
typedef _als_fixed_string<256> string256;
typedef _als_fixed_string<512> string512;
typedef _als_fixed_string<1024> string1024;
