#pragma once

#include "als/als_math.h"
#include "ecs/IComponent.h"

struct AgentComponent : public IComponent
{
    // Note: These are not stored in a Vec3 because yVelocity (gravity) is treated differently than the other velocities in the movement code.
    //       Math isn't done in the velocities as a 3D vector
    Vec2 velocity;
    float32 yVelocity = 0;

    bool isGrounded = false;
    Vec3 groundSurfaceNormal;

    static constexpr bool multipleAllowedPerEntity = false;
};
