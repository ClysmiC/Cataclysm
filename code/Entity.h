#pragma once

#include "als_math.h"

struct TransformComponent;
struct CameraComponent;
struct Ecs;

// TODO: wrap ecs.get<x>Component in functions so callers dont have to reach through pointer

struct Entity
{
	uint32 id;
    Ecs* ecs;
};

//
// Below are some common entity "bundles"
// Nothing enforces that they actually have these pointers
// set to valid values, they are simply defined to make
// code more understandable
//
struct CameraEntity : public Entity
{
	CameraEntity();
	CameraEntity(Entity entity);

	void init(Entity entity);
	
	TransformComponent *transformComponent = nullptr;
	CameraComponent *cameraComponent = nullptr;
};

