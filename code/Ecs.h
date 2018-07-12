#pragma once


#include "als_bucket_array.h"
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

struct Scene;
struct Game;

struct Ecs
{
    template<class T, uint32 BUCKET_SIZE>
    struct ComponentList
    {
        BucketArray<T, BUCKET_SIZE> components;
        std::unordered_map<uint32, ComponentGroup<T, BUCKET_SIZE>> lookup;
    };
    
    static uint32 nextEntityId;
    
    Scene* scene;

    std::vector<Entity> entities;

    static constexpr uint32 TRANSFORM_BUCKET_SIZE = 512;
    static constexpr uint32 CAMERA_BUCKET_SIZE = 8;
    static constexpr uint32 DIRECTIONAL_LIGHT_BUCKET_SIZE = 16;
    static constexpr uint32 TERRAIN_BUCKET_SIZE = 8;
    static constexpr uint32 POINT_LIGHT_BUCKET_SIZE = 32;
    static constexpr uint32 RENDER_COMPONENT_BUCKET_SIZE = 512;
    static constexpr uint32 PORTAL_BUCKET_SIZE = 16;
    static constexpr uint32 COLLIDER_BUCKET_SIZE = 512;
    static constexpr uint32 WALK_COMPONENT_BUCKET_SIZE = 8;

    ComponentList<TransformComponent,        TRANSFORM_BUCKET_SIZE>           transforms;
    ComponentList<CameraComponent,           CAMERA_BUCKET_SIZE>              cameras;
    ComponentList<DirectionalLightComponent, DIRECTIONAL_LIGHT_BUCKET_SIZE>   directionalLights;
    ComponentList<TerrainComponent,          TERRAIN_BUCKET_SIZE>             terrains;
    ComponentList<PointLightComponent,       POINT_LIGHT_BUCKET_SIZE>         pointLights;
    ComponentList<RenderComponent,           RENDER_COMPONENT_BUCKET_SIZE>    renderComponents;
    ComponentList<PortalComponent,           PORTAL_BUCKET_SIZE>              portals;
    ComponentList<ColliderComponent,         COLLIDER_BUCKET_SIZE>            colliders;
    ComponentList<WalkComponent,             WALK_COMPONENT_BUCKET_SIZE>      walkComponents;
};

//
// Component List functions
//
template<class T, uint32 BUCKET_SIZE>
T* addComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e);

template<class T, uint32 BUCKET_SIZE>
T* getComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e);

template<class T, uint32 BUCKET_SIZE>
ComponentGroup<T, BUCKET_SIZE> addComponents(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e, uint32 numComponents);

template<class T, uint32 BUCKET_SIZE>
ComponentGroup<T, BUCKET_SIZE> getComponents(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e);

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
ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> addRenderComponents(Entity e, uint32 numComponents);
ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> getRenderComponents(Entity e);

ColliderComponent* addColliderComponent(Entity e);
ColliderComponent* getColliderComponent(Entity e);
ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> addColliderComponents(Entity e, uint32 numComponents);
ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> getColliderComponents(Entity e);

PointLightComponent* addPointLightComponent(Entity e);
PointLightComponent* getPointLightComponent(Entity e);
ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> addPointLightComponents(Entity e, uint32 numComponents);
ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> getPointLightComponents(Entity e);

// TODO: do these belong in Ecs.h?
void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, Transform* cameraXfm, bool renderingViaPortal=false, Transform* destPortalXfm=nullptr);
void renderContentsOfAllPortals(Scene* scene, CameraComponent* camera, Transform* cameraXfm, uint32 recursionLevel=0);
void walkAndCamera(Game* game);

RaycastResult castRay(Ecs* ecs, Ray ray);
    
PointLightComponent* closestPointLight(TransformComponent* xfm);
