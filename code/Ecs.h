#pragma once


#include "Entity.h"
#include "Ray.h"
#include "ComponentGroup.h"
#include "TransformComponent.h"
#include "PortalComponent.h"
#include "CameraComponent.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "RenderComponent.h"
#include "ColliderComponent.h"
#include "TerrainComponent.h"
#include "WalkComponent.h"

#include <unordered_map>
#include <vector>


// TODO better solution for this.
// maybe each component type gets its own constant
#define COMPONENT_ARRAY_SIZE 512

struct Scene;
struct Game;

struct Ecs
{
    template<class T>
    struct ComponentList
    {
        T components[COMPONENT_ARRAY_SIZE];
        uint32 size = 0;
        std::unordered_map<uint32, ComponentGroup<T>> lookup;
    };
    
    static uint32 nextEntityId;
    
    Scene* scene;

    std::vector<Entity> entities;

    // TODO: make a dynamic array of pointers to contiguous "batches/buckets"
    ComponentList<TransformComponent> transforms;
    ComponentList<CameraComponent> cameras;
    ComponentList<DirectionalLightComponent> directionalLights;
    ComponentList<TerrainComponent> terrains;
    ComponentList<PointLightComponent> pointLights;
    ComponentList<RenderComponent> renderComponents;
    ComponentList<PortalComponent> portals;
    ComponentList<ColliderComponent> colliders;
    ComponentList<WalkComponent> walkComponents;
};

//
// Component List functions
//
template<class T>
T* addComponent(Ecs::ComponentList<T>* componentList, Entity e);

template<class T>
T* getComponent(Ecs::ComponentList<T>* componentList, Entity e);

template<class T>
ComponentGroup<T> addComponents(Ecs::ComponentList<T>* componentList, Entity e, uint32 numComponents);

template<class T>
ComponentGroup<T> getComponents(Ecs::ComponentList<T>* componentList, Entity e);

//
// Entity functions
//
Entity makeEntity(Ecs* ecs, string16 friendlyName="");

TransformComponent* addTransformComponent(Entity e);
TransformComponent* getTransformComponent(Entity e);

CameraComponent* addCameraComponent(Entity e);
CameraComponent* getCameraComponent(Entity e);

PortalComponent* addPortalComponent(Entity e);
PortalComponent* getPortalComponent(Entity e);

DirectionalLightComponent* addDirectionalLightComponent(Entity e);
DirectionalLightComponent* getDirectionalLightComponent(Entity e);

TerrainComponent* addTerrainComponent(Entity e);
TerrainComponent* getTerrainComponent(Entity e);

WalkComponent* addWalkComponent(Entity e);
WalkComponent* getWalkComponent(Entity e);
    
RenderComponent* addRenderComponent(Entity e);
RenderComponent* getRenderComponent(Entity e);
ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents);
ComponentGroup<RenderComponent> getRenderComponents(Entity e);

ColliderComponent* addColliderComponent(Entity e);
ColliderComponent* getColliderComponent(Entity e);
ComponentGroup<ColliderComponent> addColliderComponents(Entity e, uint32 numComponents);
ComponentGroup<ColliderComponent> getColliderComponents(Entity e);

PointLightComponent* addPointLightComponent(Entity e);
PointLightComponent* getPointLightComponent(Entity e);
ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents);
ComponentGroup<PointLightComponent> getPointLightComponents(Entity e);

// TODO: do these belong in Ecs.h?
void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, Transform* cameraXfm, bool renderingViaPortal=false, Transform* destPortalXfm=nullptr);
void renderContentsOfAllPortals(Scene* scene, CameraComponent* camera, Transform* cameraXfm, uint32 recursionLevel=0);
void walkAndCamera(Game* game);

RaycastResult castRay(Ecs* ecs, Ray ray);
    
PointLightComponent* closestPointLight(TransformComponent* xfm);
