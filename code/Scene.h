#pragma once

#include "Ecs.h"
#include "Entity.h"
#include "Cubemap.h"

struct Scene
{
	Ecs* ecs;
	Cubemap* cubemap;
	// Entity sceneEntity;

	Scene();

	void renderScene(CameraComponent* cameraComponent);
	void addCubemap(Cubemap* cubemap);
};
