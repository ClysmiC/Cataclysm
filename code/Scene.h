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

	void renderScene(CameraEntity camera);
	void addCubemap(Cubemap* cubemap);
};
