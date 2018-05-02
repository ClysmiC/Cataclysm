#pragma once

#include "Entity.h"
#include "TransformComponent.h"
#include "RenderComponent.h"

#include <unordered_map>

struct Ecs
{
	// Once you are given an Id, you can go ahead and start registering your components with it
	uint32 nextEntityId();

	TransformComponent* addTransformComponent(Entity e);
	TransformComponent* getTransformComponent(Entity e);
	
	RenderComponent* addRenderComponent(Entity e);
	RenderComponent* getRenderComponent(Entity e);
	
	void RenderAllRenderComponents();
	
private:
	uint32 nextEntityId_ = 1;
	
	std::vector<TransformComponent> transformComponents;
	std::unordered_map<Entity, TransformComponent*> transformComponentLookup;
	
	std::vector<RenderComponent> renderComponents;
	std::unordered_map<Entity, RenderComponent*> renderComponentLookup;

	template<class T>
	T* addComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> lookup);

	template<class T>
	T* getComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> lookup);
};
