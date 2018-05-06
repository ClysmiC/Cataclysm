#pragma once

#include "Entity.h"
#include "TransformComponent.h"
#include "PointLightComponent.h"
#include "RenderComponent.h"

#include <unordered_map>

#define COMPONENT_ARRAY_SIZE 512

template<class T>
struct ComponentList
{
	T components[COMPONENT_ARRAY_SIZE];
	uint32 size;
	std::unordered_map<Entity, T*> lookup;

	T* addComponent(Entity e);
	T* getComponent(Entity e);
};

struct Ecs
{
    Ecs();

	// Once you are given an Id, you can go ahead and start registering your components with it
	Entity nextEntityId();

	TransformComponent* addTransformComponent(Entity e);
	TransformComponent* getTransformComponent(Entity e);
	
	RenderComponentCollection addRenderComponents(Entity e, uint32 numComponents);
	RenderComponentCollection getRenderComponents(Entity e);

	PointLightComponent* addPointLightComponent(Entity e);
	PointLightComponent* getPointLightComponent(Entity e);

	void renderAllRenderComponents(TransformComponent* cameraXfm);
	
	PointLightComponent* closestPointLight(TransformComponent* xfm);
	
private:
	Entity nextEntityId_ = 1;

	// TODO: make a dynamic array of pointers to contiguous "batches"

	ComponentList<TransformComponent> transforms;
	ComponentList<PointLightComponent> pointLights;

	RenderComponent renderComponents[COMPONENT_ARRAY_SIZE];
	std::unordered_map<Entity, RenderComponentCollection> renderComponentLookup;
	uint32 renderComponentNextIndex = 0;
};
