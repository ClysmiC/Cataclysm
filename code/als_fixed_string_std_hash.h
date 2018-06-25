#pragma once

#include "als_fixed_string.h"
#include "Types.h"

namespace std {
    template <uint16 SIZE> struct hash<_als_fixed_string<SIZE>>
    {
        size_t operator()(const _als_fixed_string<SIZE> & str) const
        {
            // djb2 hash : http://www.cse.yorku.ca/~oz/hash.html
            size_t hash = 5381;

            for (uint16 i = 0; i < str.length; i++)
            {
                char c = str.data[i];
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
            }

            return hash;
        }
    };

     template <uint16 SIZE> struct equal_to<_als_fixed_string<SIZE>>
     {
         size_t operator()(const _als_fixed_string<SIZE> & lhs, const _als_fixed_string<SIZE> & rhs) const
         {
             // For whatever idiotic reason, if I do lhs == rhs it casts them both to char* and then
             // compares them.... instead of just calling the existing == overload.
             //
             // Just copy-pasted the implementation here... I need to figure this out and fix it though
             // or it may be a pitfall I hit later
             if (lhs.length != rhs.length) return false;

             for (uint16 i = 0; i < lhs.length; i++)
             {
                 if (lhs.data[i] != rhs.data[i]) return false;
             }

             return true;
         }
     };
}
