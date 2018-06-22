#pragma once

#include "als_math.h"

class MeshVertex
{
public:
    Vec3 position;
    Vec3 normal;
    Vec2 texCoords;
    Vec3 tangent;
    Vec3 bitangent;

    bool operator==(const MeshVertex &other)
    {
        return
            equals(this->position, other.position)    &&
            equals(this->normal, other.normal)        &&
            equals(this->texCoords, other.texCoords)  &&
            equals(this->tangent, other.tangent)      &&
            equals(this->bitangent, other.bitangent);
    }
};

