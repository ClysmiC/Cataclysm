#include "Scene.h"
#include "resource/resources/Cubemap.h"

Scene::Scene()
{
    this->ecs.scene = this;
    this->cubemap = nullptr;
}

void addCubemap(Scene* scene, Cubemap* cubemap)
{
    scene->cubemap = cubemap;
}

void renderScene(Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel, ITransform* destPortalXfm)
{
    if (recursionLevel > 3) return;
    
    if (scene->cubemap != nullptr) renderCubemap(scene->cubemap, camera, cameraXfm);

    renderContentsOfAllPortals(scene, camera, cameraXfm, recursionLevel);
    renderAllRenderComponents(&scene->ecs, camera, cameraXfm, recursionLevel > 0, destPortalXfm);
}
