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


#include "components/EntityDetails.h"
#include "components/TransformComponent.h"
#include "components/PortalComponent.h"
#include "components/CameraComponent.h"
#include "components/PointLightComponent.h"
#include "components/DirectionalLightComponent.h"
#include "components/RenderComponent.h"
#include "components/ColliderComponent.h"
#include "components/ConvexHullColliderComponent.h"
#include "components/TerrainComponent.h"
#include "components/AgentComponent.h"
#include "components/WalkComponent.h"

//struct TransformComponent;
//struct PortalComponent;
//struct CameraComponent;
//struct PointLightComponent;
//struct DirectionalLightComponent;
//struct RenderComponent;
//struct ColliderComponent;
//struct ConvexHullColliderComponent;
//struct TerrainComponent;
//struct AgentComponent;
//struct WalkComponent;

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
    template<> static constexpr uint32 ecsBucketSize<EntityDetails>()      { return Ecs::ENTITY_DETAILS_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<TransformComponent>() { return Ecs::TRANSFORM_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<CameraComponent>()    { return Ecs::CAMERA_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<DirectionalLightComponent>() { return Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<TerrainComponent>()   { return Ecs::TERRAIN_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<PointLightComponent>() { return Ecs::POINT_LIGHT_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<RenderComponent>()    { return Ecs::RENDER_COMPONENT_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<PortalComponent>()    { return Ecs::PORTAL_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<ConvexHullColliderComponent>() { return Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<ColliderComponent>()  { return Ecs::COLLIDER_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<AgentComponent>()     { return Ecs::AGENT_BUCKET_SIZE; }
    template<> static constexpr uint32 ecsBucketSize<WalkComponent>()      { return Ecs::WALK_COMPONENT_BUCKET_SIZE; }

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
T* addComponent(Entity e)
{
    if (e.id == 0) return nullptr;

    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    void* componentListPtr = e.ecs->componentListByType[typeid(T)];
    auto componentList = (Ecs::ComponentList<T, bucketSize>*)(componentListPtr);
    
    BucketLocator location = componentList->components.occupyEmptySlot();
    T* result = componentList->components.addressOf(location);
    result->entity = e;

    //
    // Creates new component group entry if one doesn't exist. Otherwise, modifies the existing one
    //
    ComponentGroup<T, bucketSize>* cg = &(componentList->lookup[e.id]);
    cg->entity = e;
    cg->bucketArray = &componentList->components;

    assert(cg->numComponents == 0 || T::multipleAllowedPerEntity);
    assert(cg->numComponents < MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY);

    cg->components[cg->numComponents] = location;
    cg->numComponents++;
    
    return result;
}

template<class T>
ComponentGroup<T, Ecs::ecsBucketSize<T>()> addComponents(Entity e, uint32 numComponents)
{
    static_assert(T::multipleAllowedPerEntity, "Multiple components of this type not allowed");
    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    ComponentGroup<T, bucketSize> result;

    if (e.id == 0)
    {
        return result;

    }

    void* componentListPtr = e.ecs->componentListByType[typeid(T)];
    auto componentList = (Ecs::ComponentList<T, bucketSize>*)(componentListPtr);
    
    //
    // Creates new component group entry if one doesn't exist. Otherwise, modifies the existing one
    //
    ComponentGroup<T, bucketSize>* componentGroup = &(componentList->lookup[e.id]);
    componentGroup->entity = e;
    componentGroup->bucketArray = &componentList->components;

    for (uint32 i = 0; i < numComponents; i++)
    {
        BucketLocator locator = componentList->components.occupyEmptySlot();
        T* component = componentList->components.addressOf(locator);
        component->entity = e;

        assert(componentGroup->numComponents < MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY);

        componentGroup->components[componentGroup->numComponents] = locator;
        componentGroup->numComponents++;
    }

    componentGroup->bucketArray = &componentList->components;

    result = *componentGroup;
    return result;
}

template<class T>
T* getComponent(Entity e)
{
    if (e.id == 0) return nullptr;

    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    const type_info& foo = typeid(T);

    void* componentListPtr = e.ecs->componentListByType[typeid(T)];
    auto componentList = (Ecs::ComponentList<T, bucketSize>*)(componentListPtr);

    auto it = componentList->lookup.find(e.id);
    if (it == componentList->lookup.end())
    {
        return nullptr;
    }

    ComponentGroup<T, bucketSize> componentGroup = it->second;
    assert(componentGroup.numComponents > 0);

    return &(componentGroup)[0];
}

template<class T>
ComponentGroup<T, Ecs::ecsBucketSize<T>()> getComponents(Entity e)
{
    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    ComponentGroup<T, bucketSize> result;
    result.numComponents = 0;

    if (e.id == 0)
    {
        return result;
    }

    static_assert(T::multipleAllowedPerEntity, "Multiple components of this type not allowed");

    void* componentListPtr = e.ecs->componentListByType[typeid(T)];
    auto componentList = (Ecs::ComponentList<T, bucketSize>*)(componentListPtr);
    
    auto it = componentList->lookup.find(e.id);
    if (it != componentList->lookup.end())
    {
        result = it->second;
    }

    return result;
}

template<class T>
bool removeComponent(T** component)
{
    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    Entity e = (*component)->entity;
    void* componentListPtr = e.ecs->componentListByType[typeid(T)];
    auto componentList = (Ecs::ComponentList<T, bucketSize>*)(componentListPtr);

    auto it = componentList->lookup.find(e.id);
    if (it == componentList->lookup.end())
    {
        return false;
    }

    ComponentGroup<T, bucketSize> &thisEntitiesComponents = it->second;

    for (uint32 i = 0; i < thisEntitiesComponents.numComponents; i++)
    {
        T* candidate = &thisEntitiesComponents[i];

        if (candidate == *component)
        {
            // Remove from bucket array
            componentList->components.remove(thisEntitiesComponents.components[i]);
                
            // Unordered remove from the lookup entry array
            thisEntitiesComponents.components[i] = thisEntitiesComponents.components[thisEntitiesComponents.numComponents - 1];
            thisEntitiesComponents.numComponents--;

            if (thisEntitiesComponents.numComponents == 0)
            {
                // Removed last of this component type for an entity, remove that entity from the
                // lookup table
                componentList->lookup.erase(it);
            }

            *component = nullptr;
            return true;
        }
    }

    return false;
}

//
// Entity functions
//
Entity makeEntity(Ecs* ecs, string16 friendlyName="", EntityFlags flags=g_defaultEntityFlags);
bool   markEntityForDeletion(Entity entity);

//EntityDetails* getEntityDetails(Entity entity);
//bool           removeEntityDetails(EntityDetails** ppComponent);
//
//TransformComponent* getComponent<TransformComponent>(Entity e);
//bool                removeTransformComponent(TransformComponent** ppComponent);
//
//CameraComponent* addComponent<CameraComponent>(Entity e);
//CameraComponent* getComponent<CameraComponent>(Entity e);
//bool             removeComponent<CameraComponent>(CameraComponent** ppComponent);
//
//PortalComponent* addComponent<PortalComponent>(Entity e);
//PortalComponent* getComponent<PortalComponent>(Entity e);
//bool             removeComponent<PortalComponent>(PortalComponent** ppComponent);
//
//DirectionalLightComponent* addComponent<DirectionalLightComponent>(Entity e);
//DirectionalLightComponent* getComponent<DirectionalLightComponent>(Entity e);
//ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE> addComponent<DirectionalLightComponent>s(Entity e, uint32 numComponents);
//ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE> getComponents<DirectionalLightComponent>(Entity e);
//bool                       removeComponent<DirectionalLightComponent>(DirectionalLightComponent** ppComponent);
//
//TerrainComponent* addTerrainComponent(Entity e);
//TerrainComponent* getComponent<TerrainComponent>(Entity e);
//bool              removeComponent<TerrainComponent>(TerrainComponent** ppComponent);
//
//AgentComponent* addComponent<AgentComponent>(Entity e);
//AgentComponent* getComponent<AgentComponent>(Entity e);
//bool              removeAgentComponent(AgentComponent** ppComponent);
//
//WalkComponent* addWalkComponent(Entity e);
//WalkComponent* getComponent<WalkComponent>(Entity e);
//bool           removeComponent<WalkComponent>(WalkComponent** ppComponent);
//    
//RenderComponent* addComponent<RenderComponent>(Entity e);
//RenderComponent* getComponent<RenderComponent>(Entity e);
//ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> addComponents<RenderComponent>(Entity e, uint32 numComponents);
//ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> getComponents<RenderComponent>(Entity e);
//bool           removeComponent<RenderComponent>(RenderComponent** ppComponent);
//
//ColliderComponent* addComponent<ColliderComponent>(Entity e);
//ColliderComponent* getComponent<ColliderComponent>(Entity e);
//ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> addComponents<ColliderComponent>(Entity e, uint32 numComponents);
//ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> getComponents<ColliderComponent>(Entity e);
//bool                                                         removeComponent<ColliderComponent>(ColliderComponent** ppComponent);
//
//ConvexHullColliderComponent* addComponent<ConvexHullColliderComponent>(Entity e);
//ConvexHullColliderComponent* getComponent<ConvexHullColliderComponent>(Entity e);
//ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE> addComponent<ConvexHullColliderComponent>s(Entity e, uint32 numComponents);
//ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE> getComponents<ConvexHullColliderComponent>(Entity e);
//bool                                                         removeComponent(ConvexHullColliderComponent** ppComponent);
//
//PointLightComponent* addComponent<PointLightComponent>(Entity e);
//PointLightComponent* getComponent<PointLightComponent>(Entity e);
//ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> addComponent<PointLightComponent>s(Entity e, uint32 numComponents);
//ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> getComponents<PointLightComponent>(Entity e);
//bool           removeComponent<PointLightComponent>(PointLightComponent** ppComponent);

// TODO: where to put this?
RaycastResult castRay(Ecs* ecs, Ray ray);
