#include "Scene.h"

Scene::Scene()
{
	ecs = new Ecs();
	ecs->scene = this;
	cubemap = nullptr;
}

void
Scene::addCubemap(Cubemap* cubemap_)
{
	cubemap = cubemap_;
}

void
Scene::renderScene(CameraComponent* camera, TransformComponent* cameraXfm, uint32 recursionLevel, TransformComponent* destPortalXfm)
{
	if (recursionLevel > 3) return;
	
	if (cubemap != nullptr) cubemap->render(camera, cameraXfm);

	ecs->renderContentsOfAllPortals(camera, cameraXfm, recursionLevel);
	ecs->renderAllRenderComponents(camera, cameraXfm, recursionLevel > 0, destPortalXfm);
}
