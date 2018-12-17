#pragma once

#include "ecs/Ecs.h"
#include "ecs/Entity.h"
#include "als/als_fixed_string.h"

struct Game;
struct Cubemap;
struct ITransform;
struct Renderer;

struct Scene
{
    Ecs ecs;
    Cubemap* cubemap;
    Game* game;
    string32 name;

    Scene();
};

RaycastResult castRay(Scene* ecs, Ray ray);
void renderScene(Renderer* renderer, Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel = 0, ITransform* destPortalXfm = nullptr);
void addCubemap(Scene* scene, Cubemap* cubemap);
