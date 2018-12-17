#pragma once

#include "Camera.h"
#include "ecs/IComponent.h"
#include "als/als_math.h"

struct Window;
struct ITransform;

struct CameraComponent : public IComponent, public ICamera
{
    static constexpr bool multipleAllowedPerEntity = false;

    ITransform* getTransform() override;
};


