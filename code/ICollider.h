#pragma once

#include "als/als_math.h"

struct ICollider
{
    bool isTrigger = false; // Triggers don't take part in collision detection
    virtual Vec3 center() = 0;
    virtual Vec3 support(Vec3 direction) = 0;
};
