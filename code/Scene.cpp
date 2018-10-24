#include "Scene.h"
#include "resource/resources/Cubemap.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/CameraComponent.h"
#include "ecs/components/PointLightComponent.h"
#include "ecs/components/DirectionalLightComponent.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/ColliderComponent.h"
#include "ecs/components/TerrainComponent.h"
#include "ecs/components/WalkComponent.h"

#include "ecs/systems/RenderSystem.h"

Scene::Scene()
{
    this->ecs.scene = this;
    this->cubemap = nullptr;
}

void addCubemap(Scene* scene, Cubemap* cubemap)
{
    scene->cubemap = cubemap;
}

void renderScene(Renderer* renderer, Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel, ITransform* destPortalXfm)
{
    if (recursionLevel > 3) return;
    
    if (scene->cubemap != nullptr) renderCubemap(scene->cubemap, camera, cameraXfm);

    // renderContentsOfAllPortals(renderer, scene, camera, cameraXfm, recursionLevel);
    renderAllRenderComponents(renderer, &scene->ecs, camera, cameraXfm, recursionLevel > 0, destPortalXfm);
}
