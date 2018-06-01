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
Scene::renderScene(CameraComponent* camera, TransformComponent* cameraXfm)
{
	if (cubemap != nullptr) cubemap->render(camera, cameraXfm);

	ecs->renderContentsOfAllPortals(camera, cameraXfm);
	ecs->renderAllRenderComponents(camera, cameraXfm);
}
