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
}
