#include "Scene.h"

Scene::Scene()
{
	ecs = new Ecs();
	cubemap = nullptr;
	// sceneEntity = ecs.makeEntity();
}

void
Scene::addCubemap(Cubemap* cubemap_)
{
	cubemap = cubemap_;
}

void
Scene::renderScene(CameraComponent* cameraComponent)
{
	if (cubemap != nullptr) cubemap->render(cameraComponent);

	ecs->renderAllRenderComponents(cameraComponent);
}
