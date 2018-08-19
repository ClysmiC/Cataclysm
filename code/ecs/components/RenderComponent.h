#pragma once

#include "resource/Submesh.h"
#include "ecs/Component.h"

struct Material;
struct ITransform;
struct CameraComponent;

struct RenderComponent : public Component
{
    RenderComponent() = default;
    RenderComponent(Entity entity, Submesh* submesh);

    Submesh* submesh;
    Material* material;

    SubmeshOpenGlInfo submeshOpenGlInfo;
    bool isVisible = true;

    static constexpr bool multipleAllowedPerEntity = true;
};

void drawRenderComponent(RenderComponent* renderComponent, ITransform *xfm,  CameraComponent* camera, ITransform *cameraXfm);
