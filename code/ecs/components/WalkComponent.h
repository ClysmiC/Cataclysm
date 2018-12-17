#pragma once

#include "ecs/IComponent.h"

struct WalkComponent : public IComponent
{
    bool isGrounded = false;
    
    Entity terrain;

    static constexpr bool multipleAllowedPerEntity = false;
};
