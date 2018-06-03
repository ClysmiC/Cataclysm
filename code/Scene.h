#pragma once

#include "Ecs.h"
#include "Entity.h"
#include "Cubemap.h"

struct Scene
{
	Ecs* ecs;
	Cubemap* cubemap;
	
	Scene();

	void renderScene(CameraComponent* camera, TransformComponent* cameraXfm, uint32 recursionLevel=0, TransformComponent* destPortalXfm=nullptr);
	void addCubemap(Cubemap* cubemap);
};
