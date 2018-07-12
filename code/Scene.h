#pragma once

#include "Ecs.h"
#include "Entity.h"
#include "Cubemap.h"
#include "als_fixed_string.h"

struct Game;

struct Scene
{
    Ecs ecs;
    Cubemap* cubemap;
    Game* game;
    string32 name;

    Scene();
};

void renderScene(Scene* scene, CameraComponent* camera, Transform* cameraXfm, uint32 recursionLevel=0, Transform* destPortalXfm=nullptr);
void addCubemap(Scene* scene, Cubemap* cubemap);
