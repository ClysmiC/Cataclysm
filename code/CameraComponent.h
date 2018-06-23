#pragma once

#include "Component.h"
#include "als_math.h"

struct Ray;
struct Window;

struct CameraComponent : public Component
{
    Mat4 projectionMatrix; // Calculated and cached. After changing a camera value, call recalculateProjectionMatrix() to update
    
    bool isOrthographic;

    float32 aspectRatio;
    float32 near;
    float32 far;

    Window* window;
    
    union
    {
        float32 perspectiveFov;
        float32 orthoWidth;
    };

    // TODO: wrappers around the math library calls
    // initPerspective(..)
    // initOrtho(..)
};

void recalculateProjectionMatrix(CameraComponent* camera);

// "pixel" space: from 0, 0 (bot left) to w - 1, h - 1 (top right)
Ray rayThroughScreenCoordinate(CameraComponent* camera, Vec2 screenCoordinate);

// viewport space: from 0, 0, (bot left) to 1, 1 (top right)
Ray rayThroughViewportCoordinate(CameraComponent* camera, Vec2 viewportCoordinate);

