#pragma once

#include "als_math.h"
#include "Mesh.h"

class Entity
{
public:
    Entity();
    ~Entity();

    Vec3 position;
    Quaternion orientation;
    Vec3 scale;

    Mesh* mesh;

    inline Mat4 transform()
    {
		Mat4 result;
        result.translateInPlace(position);
        result.rotateInPlace(orientation);
        result.scaleInPlace(scale);

        return result;
    }

	void draw();
};

