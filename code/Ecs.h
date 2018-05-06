#pragma once

#include "Entity.h"
#include "ComponentGroup.h"
#include "TransformComponent.h"
#include "PointLightComponent.h"
#include "RenderComponent.h"

#include <unordered_map>

#define COMPONENT_ARRAY_SIZE 512


struct Ecs
{
	
private:
	
	template<class T>
	struct ComponentList
	{
		T components[COMPONENT_ARRAY_SIZE];
		uint32 size = 0;
		std::unordered_map<Entity, ComponentGroup<T>> lookup;

		T* addComponent(Entity e);
		T* getComponent(Entity e);
		ComponentGroup<T> addComponents(Entity e, uint32 numComponents);
		ComponentGroup<T> getComponents(Entity e);
	};

	
public:
	
    Ecs();

	// Once you are given an Id, you can go ahead and start registering your components with it
	Entity nextEntityId();

	TransformComponent* addTransformComponent(Entity e);
	TransformComponent* getTransformComponent(Entity e);

	RenderComponent* addRenderComponent(Entity e);
	RenderComponent* getRenderComponent(Entity e);
	ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents);
	ComponentGroup<RenderComponent> getRenderComponents(Entity e);

	PointLightComponent* addPointLightComponent(Entity e);
	PointLightComponent* getPointLightComponent(Entity e);
	ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents);
	ComponentGroup<PointLightComponent> getPointLightComponents(Entity e);

	void renderAllRenderComponents(TransformComponent* cameraXfm);
	
	PointLightComponent* closestPointLight(TransformComponent* xfm);

	
private:
	
	Entity nextEntityId_ = 1;

	// TODO: make a dynamic array of pointers to contiguous "batches"

	ComponentList<TransformComponent> transforms;
	ComponentList<PointLightComponent> pointLights;
	ComponentList<RenderComponent> renderComponents;
};
