#pragma once

#include "als/als_math.h"
#include "Transform.h"
#include "Ray.h"
#include "Window.h"

struct ICamera
{
    // TODO: hide all of these behind getters/setters and auto-cache this like we do for ITransform?
    Mat4 projectionMatrix; // Calculated and cached. After changing a camera value, call recalculateProjectionMatrix() to update
    
    bool isOrthographic;

    float32 aspectRatio;
    float32 near;
    float32 far;

    Window* window; // Only needed if casting rays through the screen
    
    union
    {
        float32 perspectiveFov;
        float32 orthoWidth;
    };

    virtual ITransform* getTransform() = 0;
};

struct Camera : public ICamera
{
    Transform transform;
    ITransform* getTransform() override;
};

void recalculateProjectionMatrix(ICamera* camera);

// "pixel" space: from 0, 0 (bot left) to w - 1, h - 1 (top right)
Ray rayThroughScreenCoordinate(ICamera* camera, Vec2 screenCoordinate);

// viewport space: from 0, 0, (bot left) to 1, 1 (top right)
Ray rayThroughViewportCoordinate(ICamera* camera, Vec2 viewportCoordinate);
