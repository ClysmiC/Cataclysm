#pragma once


#include "Entity.h"
#include "ComponentGroup.h"
#include "TransformComponent.h"
#include "PortalComponent.h"
#include "CameraComponent.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "RenderComponent.h"

#include <unordered_map>


// TODO better solution for this.
// maybe each component type gets its own constant
#define COMPONENT_ARRAY_SIZE 512

struct Scene;

struct Ecs
{
	template<class T>
	struct ComponentList
	{
		T components[COMPONENT_ARRAY_SIZE];
		uint32 size = 0;
		std::unordered_map<uint32, ComponentGroup<T>> lookup;
	};

	
    Ecs();
	
	static uint32 nextEntityId;
	
	Scene* scene;

	// TODO: make a dynamic array of pointers to contiguous "batches/buckets"
	ComponentList<TransformComponent> transforms;
	ComponentList<CameraComponent> cameras;
	ComponentList<DirectionalLightComponent> directionalLights;
	ComponentList<PointLightComponent> pointLights;
	ComponentList<RenderComponent> renderComponents;
	ComponentList<PortalComponent> portals;
	


};

//
// Component List functions
//
T* addComponent(Ecs::ComponentList<T>* componentList, Entity e);
T* getComponent(Ecs::ComponentList<T>* componentList, Entity e);
ComponentGroup<T> addComponents(Ecs::ComponentList<T>* componentList, Entity e, uint32 numComponents);
ComponentGroup<T> getComponents(Ecs::ComponentList<T>* componentList, Entity e);


//
// Entity functions
//
Entity makeEntity(Ecs* ecs);

TransformComponent* addTransformComponent(Entity e);
TransformComponent* getTransformComponent(Entity e);

CameraComponent* addCameraComponent(Entity e);
CameraComponent* getCameraComponent(Entity e);

PortalComponent* addPortalComponent(Entity e);
PortalComponent* getPortalComponent(Entity e);

DirectionalLightComponent* addDirectionalLightComponent(Entity e);
DirectionalLightComponent* getDirectionalLightComponent(Entity e);
	
RenderComponent* addRenderComponent(Entity e);
RenderComponent* getRenderComponent(Entity e);
ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents);
ComponentGroup<RenderComponent> getRenderComponents(Entity e);

PointLightComponent* addPointLightComponent(Entity e);
PointLightComponent* getPointLightComponent(Entity e);
ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents);
ComponentGroup<PointLightComponent> getPointLightComponents(Entity e);

// TODO: do these belong in Ecs.h?
void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, TransformComponent* cameraXfm, bool renderingViaPortal=false, TransformComponent* destPortalXfm=nullptr);
void renderContentsOfAllPortals(Ecs* ecs, CameraComponent* camera, TransformComponent* cameraXfm, uint32 recursionLevel=0);
	
PointLightComponent* closestPointLight(TransformComponent* xfm);
