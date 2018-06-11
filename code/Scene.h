#pragma once

#include "Ecs.h"
#include "Entity.h"
#include "Cubemap.h"

struct Game;

struct Scene
{
	Ecs ecs;
	Cubemap* cubemap;
	Game* game;
	
	Scene();
};

void renderScene(Scene* scene, CameraComponent* camera, TransformComponent* cameraXfm, uint32 recursionLevel=0, TransformComponent* destPortalXfm=nullptr);
void addCubemap(Scene* scene, Cubemap* cubemap);
