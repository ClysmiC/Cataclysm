#pragma once

#include "Mesh.h"
#include "Component.h"
#include "Transform.h"
#include "CameraComponent.h"

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
