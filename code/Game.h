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
	
	Scene scenes[MAX_SCENES];
	int numScenes;
};

Scene* makeScene(Game* game);

