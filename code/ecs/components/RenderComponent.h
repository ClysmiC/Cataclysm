#pragma once

#include "resource/Submesh.h"
#include "ecs/Component.h"
#include "ecs/ComponentGroup.h"

struct Material;
struct ITransform;
struct CameraComponent;
struct Mesh;


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

template<int BUCKET_SIZE>
void initRenderComponents(ComponentGroup<RenderComponent, BUCKET_SIZE>* renderComponentGroup, Mesh* mesh)
{
    assert(renderComponentGroup->numComponents == mesh->submeshes.size());

    for(uint32 i = 0; i < mesh->submeshes.size(); i++)
    {
        RenderComponent *rc = &(*renderComponentGroup)[i];
        new (rc) RenderComponent(rc->entity, &(mesh->submeshes[i]));
    }
}