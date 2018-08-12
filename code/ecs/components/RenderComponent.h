#pragma once

#include "resource/Submesh.h"
#include "ecs/Component.h"
#include "Transform.h"
#include "CameraComponent.h"

struct Material;

struct RenderComponent : public Component
{
    RenderComponent() = default;
    RenderComponent(Entity entity, Submesh* submesh);

    Submesh* submesh;
    Material* material;

    SubmeshOpenGlInfo submeshOpenGlInfo;

    static constexpr bool multipleAllowedPerEntity = true;
};

void drawRenderComponent(RenderComponent* renderComponent, ITransform *xfm,  CameraComponent* camera, ITransform *cameraXfm);
