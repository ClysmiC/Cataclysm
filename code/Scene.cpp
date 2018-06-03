#include "Scene.h"

Scene::Scene()
{
	ecs = new Ecs();
	ecs->scene = this;
	cubemap = nullptr;
}

void addCubemap(Scene* scene, Cubemap* cubemap)
{
	scene->cubemap = cubemap;
}

void renderScene(Scene* scene, CameraComponent* camera, TransformComponent* cameraXfm, uint32 recursionLevel, TransformComponent* destPortalXfm)
{
	if (recursionLevel > 3) return;
	
	if (scene->cubemap != nullptr) renderCubemap(scene->cubemap, camera, cameraXfm);

	renderContentsOfAllPortals(scene->ecs, camera, cameraXfm, recursionLevel);
	renderAllRenderComponents(scene->ecs, camera, cameraXfm, recursionLevel > 0, destPortalXfm);
}
