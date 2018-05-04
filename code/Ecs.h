#pragma once

#include "Entity.h"
#include "TransformComponent.h"
#include "RenderComponent.h"

#include <unordered_map>

#define COMPONENT_ARRAY_SIZE 512

struct Ecs
{
    Ecs();

	// Once you are given an Id, you can go ahead and start registering your components with it
	Entity nextEntityId();

	TransformComponent* addTransformComponent(Entity e);
	TransformComponent* getTransformComponent(Entity e);
	
	RenderComponentCollection addRenderComponents(Entity e, uint32 numComponents);
	RenderComponentCollection getRenderComponents(Entity e);
	
	void renderAllRenderComponents(Camera &camera);
	
private:
	Entity nextEntityId_ = 1;


	// TODO: make a dynamic array of pointers to contiguous "batches"
	TransformComponent transformComponents[COMPONENT_ARRAY_SIZE];
	std::unordered_map<Entity, TransformComponent*> transformComponentLookup;
	uint32 transformComponentNextIndex = 0;

	RenderComponent renderComponents[COMPONENT_ARRAY_SIZE];
	std::unordered_map<Entity, RenderComponentCollection> renderComponentLookup;
	uint32 renderComponentNextIndex = 0;

	template<class T>
	T* addComponent(Entity e, T* components, uint32 &index, std::unordered_map<Entity, T*> &lookup);

	template<class T>
	T* getComponent(Entity e, std::unordered_map<Entity, T*> &lookup);
};
