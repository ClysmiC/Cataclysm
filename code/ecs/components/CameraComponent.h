#pragma once

#include "Camera.h"
#include "ecs/Component.h"
#include "als/als_math.h"

struct Window;
struct ITransform;

struct CameraComponent : public Component, public ICamera
{
    static constexpr bool multipleAllowedPerEntity = false;

    ITransform* getTransform() override;
};


