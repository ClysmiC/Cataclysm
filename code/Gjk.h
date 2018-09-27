#pragma once

#include "als/als_math.h"
#include "ICollider.h"

struct GjkResult
{
    bool collides;
    Vec3 penetrationVector;

    GjkResult() : collides(false), penetrationVector(0, 0, 0) {}
};

GjkResult gjk(ICollider* a, ICollider* b, bool calculatePenetrationVector=true);
