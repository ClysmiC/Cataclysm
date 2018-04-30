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
		// TODO:
        // QMatrix4x4 result;
        // result.translate(position);
        // result.rotate(orientation);
        // result.scale(scale);

        return result;
    }

	void draw();
};

