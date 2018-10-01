#include "Camera.h"
#include "ecs/Ecs.h"
#include "Window.h"
#include "Ray.h"

ITransform* Camera::getTransform()
{
    return &this->transform;
}

void recalculateProjectionMatrix(ICamera* camera)
{
    if (camera->isOrthographic)
    {
        camera->projectionMatrix.orthoInPlace(camera->orthoWidth, camera->aspectRatio, camera->near, camera->far);
    }
    else
    {
        camera->projectionMatrix.perspectiveInPlace(camera->perspectiveFov, camera->aspectRatio, camera->near, camera->far);
    }
}

// "pixel" space: from 0, 0 (bot left) to w - 1, h - 1 (top right)
Ray rayThroughScreenCoordinate(ICamera* camera, Vec2 screenCoordinate)
{
    return rayThroughViewportCoordinate(camera, pixelToViewportCoordinate(camera->window, screenCoordinate));
}

// viewport space: from 0, 0, (bot left) to 1, 1 (top right)
Ray rayThroughViewportCoordinate(ICamera* camera, Vec2 viewportCoordinate)
{
    clamp(viewportCoordinate.x, 0, 1);
    clamp(viewportCoordinate.y, 0, 1);
    
    Ray result;

    ITransform* cameraXfm = camera->getTransform();
    Vec3 cameraForward = cameraXfm->forward();
    Vec3 cameraRight = cameraXfm->right();
    Vec3 cameraUp = cameraXfm->up();
        
    Vec3 nearPlaneCenter = cameraXfm->position() + cameraForward * camera->near;
    
    if (camera->isOrthographic)
    {
        float32 halfWidth = camera->orthoWidth / 2.0f;
        float32 halfHeight = halfWidth / camera->aspectRatio;
        
        Vec3 nearPlaneBottomLeft = nearPlaneCenter - cameraRight * halfWidth - cameraUp * halfHeight;
        Vec3 nearPlaneTopRight = nearPlaneCenter + cameraRight * halfWidth + cameraUp * halfHeight;
        Vec3 nearPlaneBlToTr = nearPlaneTopRight - nearPlaneBottomLeft;

        float32 nearPlaneWorldSpaceWidth = length(project(nearPlaneBlToTr, cameraRight));
        float32 nearPlaneWorldSpaceHeight = nearPlaneWorldSpaceWidth / camera->aspectRatio;

        result.position =
            nearPlaneBottomLeft +
            viewportCoordinate.x * nearPlaneWorldSpaceWidth * cameraRight +
            viewportCoordinate.y * nearPlaneWorldSpaceHeight * cameraUp;

        result.direction = cameraForward;
    }
    else
    {
        float32 halfFov = camera->perspectiveFov / 2.0f;
        float32 halfWidth = camera->near * tanf(TO_RAD(halfFov));
        float32 halfHeight = halfWidth / camera->aspectRatio;
        
        Vec3 nearPlaneBottomLeft = nearPlaneCenter - cameraRight * halfWidth - cameraUp * halfHeight;
        Vec3 nearPlaneTopRight = nearPlaneCenter + cameraRight * halfWidth + cameraUp * halfHeight;
        Vec3 nearPlaneBlToTr = nearPlaneTopRight - nearPlaneBottomLeft;

        float32 nearPlaneWorldSpaceWidth = length(project(nearPlaneBlToTr, cameraRight));
        float32 nearPlaneWorldSpaceHeight = nearPlaneWorldSpaceWidth / camera->aspectRatio;

        result.position =
            nearPlaneBottomLeft +
            viewportCoordinate.x * nearPlaneWorldSpaceWidth * cameraRight +
            viewportCoordinate.y * nearPlaneWorldSpaceHeight * cameraUp;

        result.direction = (result.position - cameraXfm->position()).normalizeInPlace();
    }

    return result;
}
