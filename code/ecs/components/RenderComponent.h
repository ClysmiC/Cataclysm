#pragma once

#include "resource/Submesh.h"
#include "ecs/IComponent.h"
#include "ecs/ComponentGroup.h"
#include "resource/resources/Mesh.h"

struct Material;
struct ITransform;
struct ICamera;
struct Shader;

struct RenderComponent : public IComponent
{
    RenderComponent() = default;
    RenderComponent(Entity entity, Submesh* submesh);

    Submesh* submesh;
    Material* material;

    SubmeshOpenGlInfo submeshOpenGlInfo;
    bool isVisible = true;

    static constexpr bool multipleAllowedPerEntity = true;
};

void drawRenderComponent(RenderComponent* renderComponent, ITransform *xfm,  ICamera* camera, ITransform *cameraXfm, uint32 shadowMapTextureId, Mat4& lightMatrix);
void drawRenderComponentWithBoundShader(RenderComponent* renderComponent);

Mesh* getMesh(Entity e);

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
