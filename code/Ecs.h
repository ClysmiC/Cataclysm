#pragma once


#include "Entity.h"
#include "ComponentGroup.h"
#include "TransformComponent.h"
#include "PortalComponent.h"
#include "CameraComponent.h"
#include "PointLightComponent.h"
#include "RenderComponent.h"

#include <unordered_map>


// TODO better solution for this.
// maybe each component type gets its own constant
#define COMPONENT_ARRAY_SIZE 512

struct Ecs
{
	
private:
	
	template<class T>
	struct ComponentList
	{
		T components[COMPONENT_ARRAY_SIZE];
		uint32 size = 0;
		std::unordered_map<uint32, ComponentGroup<T>> lookup;

		T* addComponent(Entity e, Ecs* ecs);
		T* getComponent(Entity e);
		ComponentGroup<T> addComponents(Entity e, uint32 numComponents, Ecs* ecs);
		ComponentGroup<T> getComponents(Entity e);
	};

	
public:
	
    Ecs();

	// Once you are given an Id, you can go ahead and start registering your components with it
	Entity makeEntity();

	TransformComponent* addTransformComponent(Entity e);
	TransformComponent* getTransformComponent(Entity e);

	CameraComponent* addCameraComponent(Entity e);
	CameraComponent* getCameraComponent(Entity e);

	PortalComponent* addPortalComponent(Entity e);
	PortalComponent* getPortalComponent(Entity e);

	RenderComponent* addRenderComponent(Entity e);
	RenderComponent* getRenderComponent(Entity e);
	ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents);
	ComponentGroup<RenderComponent> getRenderComponents(Entity e);

	PointLightComponent* addPointLightComponent(Entity e);
	PointLightComponent* getPointLightComponent(Entity e);
	ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents);
	ComponentGroup<PointLightComponent> getPointLightComponents(Entity e);

	void renderAllRenderComponents(CameraEntity camera);
	void renderContentsOfAllPortals(CameraEntity camera);
	
	PointLightComponent* closestPointLight(TransformComponent* xfm);

	
private:
	
	static uint32 nextEntityId;

	// TODO: make a dynamic array of pointers to contiguous "batches/buckets"

	ComponentList<TransformComponent> transforms;
	ComponentList<CameraComponent> cameras;
	ComponentList<PointLightComponent> pointLights;
	ComponentList<RenderComponent> renderComponents;
	ComponentList<PortalComponent> portals;
};
