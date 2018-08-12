#pragma once

#include "Component.h"

struct WalkComponent : public Component
{
    bool isGrounded = false;
    
    Entity terrain;

    static constexpr bool multipleAllowedPerEntity = false;
};
