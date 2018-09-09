#pragma once

#include "als/als_math.h"

struct ICollider
{
    virtual Vec3 center() = 0;
    virtual Vec3 support(Vec3 direction) = 0;
};
