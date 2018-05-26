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
Scene::renderScene(CameraEntity camera)
{
	if (cubemap != nullptr) cubemap->render(camera);

	ecs->renderAllRenderComponents(camera);
}
