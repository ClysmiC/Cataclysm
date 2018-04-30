#pragma once

#include "Entity.h"

class PointLight : public Entity
{
public:
    PointLight();
    ~PointLight();

    Vec3 intensity;
};

