#pragma once

#include "Types.h"

template <uint16 SIZE> struct _als_fixed_string
{
    // =========================
    //
    // Members
    //
    // =========================
    uint16 length;
    char data[SIZE];
    char _safety_byte; // always set to '\0' so that a pointer to a string of length SIZE will still have a null terminator
    bool isTruncated;

    // =========================
    //
    // Constructors
    //
    // =========================

    //
    // Default
    //
    _als_fixed_string()
    {
        length = 0;
        data[0] = '\0';
        _safety_byte = '\0';
        isTruncated = false;
    }

    //
    // From c-string
    //
    _als_fixed_string(const char* cstr)
        : _als_fixed_string()
    {        
        for (int i = 0; i < SIZE; i++)
        {
            char c = cstr[i];
            data[i] = c;
            
            if (c == '\0') break;

            length++;
        }

        if (length == SIZE && cstr[length] != '\0')
        {
            isTruncated = true;
        }
    }
    
    //
    // From fixed string of different size
    //
    template <uint16 INPUT_SIZE>
    explicit _als_fixed_string<SIZE>(const _als_fixed_string<INPUT_SIZE>& input)
        : _als_fixed_string()
    {

        for (uint16 i = 0; i < input.length; i++)
        {
            if (i >= SIZE)
            {
                this->isTruncated = true;
                break;
            }
            
            this->data[i] = input.data[i];
            this->length++;
        }

        this->data[this->length] = '\0';
    }

    // =========================
    //
    // Operator overloads
    //
    // =========================

    //
    // Assignment from fixed string of different size
    //
    template <uint16 RHS_SIZE>
    _als_fixed_string<SIZE>& operator = (const _als_fixed_string<RHS_SIZE>& rhs)
    {
        new (this) _als_fixed_string(rhs);
        return *this;
    }

    //
    // String equality test (char*)
    //
    bool operator == (const char* rhs)
    {
        if (rhs[this->length] != '\0') return false;
        
        for (uint16 i = 0; i < this->length; i++)
        {
            char c = rhs[i];
            if (this->data[i] != c) return false;
            if (c == '\0')          return false;
        }

        return true;
    }

    //
    // String inequality test
    //
    template <uint16 RHS_SIZE>
    bool operator != (const _als_fixed_string<RHS_SIZE>& rhs)
    {
        return !(*this == rhs);
    }

    bool operator != (const _als_fixed_string& rhs)
    {
        return !(*this == rhs);
    }

    bool operator != (const char* rhs)
    {
        return !(*this == rhs);
    }

    //
    // TODO: String comparison tests
    //

    // =========================
    //
    // Conversions
    //
    // =========================

    //
    // Explicit conversion to const char* (c-string)
    //
    const char* cstr() const
    {
        return this->data;
    }

    //
    // Implicit conversion to different sized fixed-string
    //
    template <uint16 TARGET_SIZE>
    operator _als_fixed_string<TARGET_SIZE> () const
    {
        _als_fixed_string<TARGET_SIZE> result;

        for (uint16 i = 0; i < this->length; i++)
        {
            if (i >= SIZE)
            {
                result.isTruncated = true;
                break;
            }
            
            result.data[i] = this->data[i];
            result.length++;
        }

        result.data[result.length] = '\0';

        return result;
    }

    char operator[] (int i)
    {
        return this->data[i];
    }

    _als_fixed_string substring(uint16 startIndex)
    {
        return this->substring(startIndex, this->length);
    }
    
    _als_fixed_string substring(uint16 startIndex, uint16 endIndex)
    {
        if (endIndex > SIZE) endIndex = SIZE;
        _als_fixed_string result;
        if (startIndex >= endIndex) return result;
        
        for (uint16 resultIndex = 0, i = startIndex; i < endIndex; i++, resultIndex++)
        {
            result.data[resultIndex] = this->data[i];
            result.length++;
        }

        result.data[result.length] = '\0';

        return result;
    }

    int32 lastIndexOf(char c)
    {
        for (uint16 i = this->length - 1; i >= 0; i--)
        {
            if(this->data[i] == c) return i;
        }

        return -1;
    }

    int32 indexOf(char c)
    {
        for (uint16 i = 0; i < this->length; i++)
        {
            if(this->data[i] == c) return i;
        }

        return -1;
    }
};

constexpr uint16 _myUInt16Max(const uint16 lhs, const uint16 rhs)
{
    return lhs >= rhs ? lhs : rhs;
}

//
// String equality test (different sized fixed string)
//
template <uint16 LHS_SIZE, uint16 RHS_SIZE>
bool operator == (const _als_fixed_string<LHS_SIZE>& lhs, const _als_fixed_string<RHS_SIZE>& rhs)
{
    if (lhs.length != rhs.length) return false;

    for (uint16 i = 0; i < lhs.length; i++)
    {
        if (lhs.data[i] != rhs.data[i]) return false;
    }

    return true;
}

//
// Concatenates two fixed strings.
// The resulting type is 2x the type of the larger of the two fixed string types.
// Example: string32 + string16 would return a string64
//
// Although this can quickly grow to an unnecessarily large string, the idea is that there should only be a few concatenations,
// and then the end result gets truncated when it is assigned to a known string size.
//
// The alternative would be to have the resulting type of the + operator be the larger of the two fixed string types,
// but then scenarios like string256 = string128 + string128 would cause truncation, because the result of the + operation
// would be a too small (i.e., string128)
//
template <uint16 LHS_SIZE, uint16 RHS_SIZE>
_als_fixed_string<2 * _myUInt16Max(LHS_SIZE, RHS_SIZE)> operator + (const _als_fixed_string<LHS_SIZE> &lhs, const _als_fixed_string<RHS_SIZE> &rhs)
{
    constexpr uint16 resultSize = 2 * _myUInt16Max(LHS_SIZE, RHS_SIZE);
    _als_fixed_string<resultSize> result;

   for (uint16 i = 0; i < lhs.length; i++)
   {
       result.data[i] = lhs.data[i];
   }

   for (uint16 i = 0; i < rhs.length; i++)
   {
       uint16 index = lhs.length + i;
       result.data[index] = rhs.data[i];
   }

   result.length = lhs.length + rhs.length;

   // NOTE: if result.length = resultSize, this is writing \0 into the safety byte
   // (which is fine... just be aware of it)
   result.data[result.length] = '\0';

   return result;
}

//
// Appends a fixed string to an existing fixed string. If the modified string overflows, truncate it.
//
template <uint16 LHS_SIZE, uint16 RHS_SIZE>
_als_fixed_string<LHS_SIZE>& operator += (_als_fixed_string<LHS_SIZE> &lhs, const _als_fixed_string<RHS_SIZE> &rhs)
{
    for (uint16 i = 0; i < rhs.length; i++)
    {
        if (lhs.length >= LHS_SIZE)
        {
            lhs.isTruncated = true;
            break;
        }
        
        lhs.data[lhs.length] = rhs.data[i];
        lhs.length++;
    }

    // NOTE: if lhs.length = SIZE, this is writing \0 into the safety byte
    // (which is fine... just be aware of it)
    lhs.data[lhs.length] = '\0';

    return lhs;
}

//
// Appends a c-string to an existing fixed string. If the modified string overflows, truncate it.
//
template<uint16 SIZE>
_als_fixed_string<SIZE>& operator += (_als_fixed_string<SIZE> &lhs, const char* rhs)
{
    uint16 rhsIndex = 0;
    for (uint16 lhsIndex = lhs.length; lhsIndex < SIZE; lhsIndex++, rhsIndex++)
    {
        char c = rhs[rhsIndex];
        lhs.data[lhsIndex] = c;

        if (c == '\0') break;
        
        lhs.length++;
    }

    if (lhs.length == SIZE && rhs[rhsIndex] != '\0')
    {
        lhs.isTruncated = true;
    }

    return lhs;
}

//
// Appends a char to an existing fixed string. If the modified string overflows, truncate it.
//
template<uint16 SIZE>
_als_fixed_string<SIZE>& operator += (_als_fixed_string<SIZE> &lhs, const char rhs)
{
    const char charAndNullPtr[2] = { rhs, '\0' };
    lhs += charAndNullPtr;
    return lhs;
}


//
// TODO: these can truncate.... not sure how I feel about that. But since char* can be indefinite,
// we can't guarantee at compile-time that we return a fixed-size big enough to handle it, like
// we can with the case that we are adding two fixed-sized strings.
//
// Should it just double in size to make the likelyhood of a truncate much lower? It still wouldn't
// be guaranteed though.

//
// Concatenate char* with fixed stirng
//
template<uint16 SIZE>
_als_fixed_string<SIZE> operator + (const char* lhs, const _als_fixed_string<SIZE> &rhs)
{
    _als_fixed_string<SIZE> result = lhs;
    result += rhs;
    return result;
}

//
// Concatenate fixed string with char*
//
template<uint16 SIZE>
_als_fixed_string<SIZE> operator + (const _als_fixed_string<SIZE>& lhs, const char* rhs)
{
    _als_fixed_string<SIZE> result = lhs;
    result += rhs;
    return result;
}

//
// Concatenate fixed string with char
//
template<uint16 SIZE>
_als_fixed_string<SIZE> operator + (const _als_fixed_string<SIZE>& lhs, const char rhs)
{
    const char charAndNullPtr[2] = { rhs, '\0' };
    _als_fixed_string<SIZE> result = lhs;
    result += charAndNullPtr;
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
