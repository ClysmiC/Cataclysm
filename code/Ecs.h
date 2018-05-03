#pragma once

#include "Entity.h"
#include "TransformComponent.h"
#include "RenderComponent.h"

#include <unordered_map>

struct Ecs
{
    Ecs();

	// Once you are given an Id, you can go ahead and start registering your components with it
	Entity nextEntityId();

	TransformComponent* addTransformComponent(Entity e);
	TransformComponent* getTransformComponent(Entity e);
	
	RenderComponentCollection addRenderComponents(Entity e, uint32 numComponents);
	RenderComponentCollection getRenderComponents(Entity e);
	
	void RenderAllRenderComponents(Camera &camera);
	
private:
	Entity nextEntityId_ = 1;
	
	std::vector<TransformComponent> transformComponents;
	std::unordered_map<Entity, TransformComponent*> transformComponentLookup;
	
	std::vector<RenderComponent> renderComponents;
	std::unordered_map<Entity, RenderComponentCollection> renderComponentLookup;

	template<class T>
	T* addComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> &lookup);

	template<class T>
	T* getComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> &lookup);
};
