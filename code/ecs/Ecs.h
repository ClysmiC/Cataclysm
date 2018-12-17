#pragma once

#include "als/als_bucket_array.h"
#include "Entity.h"
#include "ecs/components/EntityDetails.h"
#include "Ray.h"
#include "ComponentGroup.h"

#include <typeindex>
#include <unordered_map>
#include <vector>

struct ITransform;
struct Scene;
struct Game;

struct TransformComponent;
struct PortalComponent;
struct CameraComponent;
struct PointLightComponent;
struct DirectionalLightComponent;
struct RenderComponent;
struct ColliderComponent;
struct ConvexHullColliderComponent;
struct TerrainComponent;
struct AgentComponent;
struct WalkComponent;

struct Ecs
{
    template<class T, uint32 BUCKET_SIZE>
    struct ComponentList
    {
        BucketArray<T, BUCKET_SIZE> components;
        std::unordered_map<uint32, ComponentGroup<T, BUCKET_SIZE>> lookup;

        uint32 count() { return components.count; }

        T& operator [] (uint32 index)
        {
            return *(components.at(index));
        }
    };

    Ecs();
    
    static uint32 nextEntityId;
    
    Scene* scene;
    Game* game;

    static constexpr uint32 ENTITY_DETAILS_BUCKET_SIZE = 512;
    static constexpr uint32 TRANSFORM_BUCKET_SIZE = 512;
    static constexpr uint32 CAMERA_BUCKET_SIZE = 4;
    static constexpr uint32 DIRECTIONAL_LIGHT_BUCKET_SIZE = 16;
    static constexpr uint32 TERRAIN_BUCKET_SIZE = 8;
    static constexpr uint32 POINT_LIGHT_BUCKET_SIZE = 32;
    static constexpr uint32 RENDER_COMPONENT_BUCKET_SIZE = 512;
    static constexpr uint32 PORTAL_BUCKET_SIZE = 16;
    static constexpr uint32 CONVEX_HULL_COLLIDER_BUCKET_SIZE = 512;
    static constexpr uint32 COLLIDER_BUCKET_SIZE = 512;
    static constexpr uint32 AGENT_BUCKET_SIZE = 512;
    static constexpr uint32 WALK_COMPONENT_BUCKET_SIZE = 8;

    // Compile-time lookup for a specific component's bucket size
    template<typename T> static constexpr uint32 ecsBucketSize() { return T::UNIMPLEMENTED_FUNCTION; } // only the specialized templates should be called

    std::vector<Entity> entities;
    ComponentList<EntityDetails,               ENTITY_DETAILS_BUCKET_SIZE>         entityDetails;
    ComponentList<TransformComponent,          TRANSFORM_BUCKET_SIZE>              transforms;
    ComponentList<CameraComponent,             CAMERA_BUCKET_SIZE>                 cameras;
    ComponentList<DirectionalLightComponent,   DIRECTIONAL_LIGHT_BUCKET_SIZE>      directionalLights;
    ComponentList<TerrainComponent,            TERRAIN_BUCKET_SIZE>                terrains;
    ComponentList<PointLightComponent,         POINT_LIGHT_BUCKET_SIZE>            pointLights;
    ComponentList<RenderComponent,             RENDER_COMPONENT_BUCKET_SIZE>       renderComponents;
    ComponentList<PortalComponent,             PORTAL_BUCKET_SIZE>                 portals;
    ComponentList<ColliderComponent,           COLLIDER_BUCKET_SIZE>               colliders;
    ComponentList<ConvexHullColliderComponent, CONVEX_HULL_COLLIDER_BUCKET_SIZE>   convexHullColliders;
    ComponentList<AgentComponent,              AGENT_BUCKET_SIZE>                  agentComponents;
    ComponentList<WalkComponent,               WALK_COMPONENT_BUCKET_SIZE>         walkComponents;

    std::unordered_map<std::type_index, void*> componentListByType;
};

template<class T>
T* addComponent(Entity e);

template<class T>
T* getComponent(Entity e);

template<class T>
ComponentGroup<T, Ecs::ecsBucketSize<T>()> addComponents(Entity e, uint32 numComponents);

template<class T>
ComponentGroup<T, Ecs::ecsBucketSize<T>()> getComponents(Entity e);

template<class T>
bool removeComponent(T* component);

//
// Entity functions
//
Entity makeEntity(Ecs* ecs, string16 friendlyName="", EntityFlags flags=g_defaultEntityFlags);
bool   markEntityForDeletion(Entity entity);

EntityDetails* getEntityDetails(Entity entity);
bool           removeEntityDetails(EntityDetails** ppComponent);

TransformComponent* getTransformComponent(Entity e);
bool                removeTransformComponent(TransformComponent** ppComponent);

CameraComponent* addCameraComponent(Entity e);
CameraComponent* getCameraComponent(Entity e);
bool             removeCameraComponent(CameraComponent** ppComponent);

PortalComponent* addPortalComponent(Entity e);
PortalComponent* getPortalComponent(Entity e);
bool             removePortalComponent(PortalComponent** ppComponent);

DirectionalLightComponent* addDirectionalLightComponent(Entity e);
DirectionalLightComponent* getDirectionalLightComponent(Entity e);
ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE> addDirectionalLightComponents(Entity e, uint32 numComponents);
ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE> getDirectionalLightComponents(Entity e);
bool                       removeDirectionalLightComponent(DirectionalLightComponent** ppComponent);

TerrainComponent* addTerrainComponent(Entity e);
TerrainComponent* getTerrainComponent(Entity e);
bool              removeTerrainComponent(TerrainComponent** ppComponent);

AgentComponent* addAgentComponent(Entity e);
AgentComponent* getAgentComponent(Entity e);
bool              removeAgentComponent(AgentComponent** ppComponent);

WalkComponent* addWalkComponent(Entity e);
WalkComponent* getWalkComponent(Entity e);
bool           removeWalkComponent(WalkComponent** ppComponent);
    
RenderComponent* addRenderComponent(Entity e);
RenderComponent* getRenderComponent(Entity e);
ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> addRenderComponents(Entity e, uint32 numComponents);
ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> getRenderComponents(Entity e);
bool           removeRenderComponent(RenderComponent** ppComponent);

ColliderComponent* addColliderComponent(Entity e);
ColliderComponent* getColliderComponent(Entity e);
ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> addColliderComponents(Entity e, uint32 numComponents);
ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> getColliderComponents(Entity e);
bool                                                         removeColliderComponent(ColliderComponent** ppComponent);

ConvexHullColliderComponent* addConvexHullColliderComponent(Entity e);
ConvexHullColliderComponent* getConvexHullColliderComponent(Entity e);
ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE> addConvexHullColliderComponents(Entity e, uint32 numComponents);
ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE> getConvexHullColliderComponents(Entity e);
bool                                                         removeConvexHullColliderComponent(ConvexHullColliderComponent** ppComponent);

PointLightComponent* addPointLightComponent(Entity e);
PointLightComponent* getPointLightComponent(Entity e);
ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> addPointLightComponents(Entity e, uint32 numComponents);
ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> getPointLightComponents(Entity e);
bool           removePointLightComponent(PointLightComponent** ppComponent);

// TODO: where to put this?
RaycastResult castRay(Ecs* ecs, Ray ray);
