#pragma once

#include "Scene.h"

extern float mouseX;
extern float mouseY;
extern float mouseXPrev;
extern float mouseYPrev;

extern bool keys[1024];
extern bool lastKeys[1024];

#define MAX_SCENES 8

struct Game
{
	Game() = default;

	// Any component that is not confined to a single scene should be in the "global" scene's ECS
	Scene global;

	// Any component confined to a scene in this array's ECS.
	// Only 1 (or maybe a few) of these scenes will be active at a time
	Scene scenes[MAX_SCENES];

	int numScenes;
};

Scene* makeScene(Game* game);

