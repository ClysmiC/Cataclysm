#include "Ecs.h"
#include "Scene.h"
#include "assert.h"

#include "Game.h"

#include "Quad.h"
#include "DebugDraw.h"

#include "resource/resources/Shader.h"
#include "Transform.h"

#include "Game.h"
#include <string>

#include "imgui/imgui.h"

#include "als/als_util.h"

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

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

// Compile time lookups
template<> constexpr uint32 Ecs::ecsBucketSize<EntityDetails>()      { return Ecs::ENTITY_DETAILS_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<TransformComponent>() { return Ecs::TRANSFORM_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<CameraComponent>()    { return Ecs::CAMERA_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<DirectionalLightComponent>() { return Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<TerrainComponent>()   { return Ecs::TERRAIN_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<PointLightComponent>() { return Ecs::POINT_LIGHT_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<RenderComponent>()    { return Ecs::RENDER_COMPONENT_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<PortalComponent>()    { return Ecs::PORTAL_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<ConvexHullColliderComponent>() { return Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<ColliderComponent>()  { return Ecs::COLLIDER_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<AgentComponent>()     { return Ecs::AGENT_BUCKET_SIZE; }
template<> constexpr uint32 Ecs::ecsBucketSize<WalkComponent>()      { return Ecs::WALK_COMPONENT_BUCKET_SIZE; }

//template<> auto Ecs::ecsComponentListAddress<EntityDetails>() { return &this->entityDetails; }
//template<> Ecs::ComponentList<TransformComponent, Ecs::ecsBucketSize<TransformComponent>()>* Ecs::ecsComponentListAddress<TransformComponent>() { return &this->transforms; }
//template<> Ecs::ComponentList<CameraComponent, Ecs::ecsBucketSize<CameraComponent>()>* Ecs::ecsComponentListAddress<CameraComponent>() { return &this->cameras; }
//template<> Ecs::ComponentList<DirectionalLightComponent, Ecs::ecsBucketSize<DirectionalLightComponent>()>* Ecs::ecsComponentListAddress<DirectionalLightComponent>() { return &this->directionalLights; }
//template<> Ecs::ComponentList<TerrainComponent, Ecs::ecsBucketSize<TerrainComponent>()>* Ecs::ecsComponentListAddress<TerrainComponent>() { return &this->terrains; }
//template<> Ecs::ComponentList<PointLightComponent, Ecs::ecsBucketSize<PointLightComponent>()>* Ecs::ecsComponentListAddress<PointLightComponent>() { return &this->pointLights; }
//template<> Ecs::ComponentList<RenderComponent, Ecs::ecsBucketSize<RenderComponent>()>* Ecs::ecsComponentListAddress<RenderComponent>() { return &this->renderComponents; }
//template<> Ecs::ComponentList<PortalComponent, Ecs::ecsBucketSize<PortalComponent>()>* Ecs::ecsComponentListAddress<PortalComponent>() { return &this->portals; }
//template<> Ecs::ComponentList<ConvexHullColliderComponent, Ecs::ecsBucketSize<ConvexHullColliderComponent>()>* Ecs::ecsComponentListAddress<ConvexHullColliderComponent>() { return &this->convexHullColliders; }
//template<> Ecs::ComponentList<ColliderComponent, Ecs::ecsBucketSize<ColliderComponent>()>* Ecs::ecsComponentListAddress<ColliderComponent>() { return &this->colliders; }
//template<> Ecs::ComponentList<AgentComponent, Ecs::ecsBucketSize<AgentComponent>()>* Ecs::ecsComponentListAddress<AgentComponent>() { return &this->agentComponents; }
//template<> Ecs::ComponentList<WalkComponent, Ecs::ecsBucketSize<WalkComponent>()>* Ecs::ecsComponentListAddress<WalkComponent>() { return &this->walkComponents; }

template<class T, uint32 BUCKET_SIZE>
T* _addComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
{
    BucketLocator location = componentList->components.occupyEmptySlot();
    T* result = componentList->components.addressOf(location);
    result->entity = e;

    //
    // Creates new component group entry if one doesn't exist. Otherwise, modifies the existing one
    //
    ComponentGroup<T, BUCKET_SIZE>* cg = &(componentList->lookup[e.id]);
    cg->entity = e;
    cg->bucketArray = &componentList->components;

    assert(cg->numComponents == 0 || T::multipleAllowedPerEntity);
    assert(cg->numComponents < MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY);

    cg->components[cg->numComponents] = location;
    cg->numComponents++;
    
    return result;    
}

template<class T>
T* addComponent(Entity e)
{
    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    void* ptr = e.ecs->componentListByType[typeid(T)];
    auto ptrCasted = (Ecs::ComponentList<T, bucketSize>*)(ptr);

    return _addComponent<T, bucketSize>(ptrCasted, e);
}

template<class T, uint32 BUCKET_SIZE>
T* _getComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
{
    auto it = componentList->lookup.find(e.id);
    if (it == componentList->lookup.end())
    {
        return nullptr;
    }

    ComponentGroup<T, BUCKET_SIZE> cg = it->second;
    assert(cg.numComponents > 0);

    return &(cg)[0];
}

template<class T>
T* getComponent(Entity e)
{
    auto constexpr bucketSize = Ecs::ecsBucketSize<T>();

    void* ptr = e.ecs->componentListByType[typeid(T)];
    auto ptrCasted = (Ecs::ComponentList<T, bucketSize>*)(ptr);

    return _getComponent<T, bucketSize>(ptrCasted, e);
}

template<class T, uint32 BUCKET_SIZE>
ComponentGroup<T, BUCKET_SIZE> addComponents(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e, uint32 numComponents)
{
    assert(T::multipleAllowedPerEntity);
    
    //
    // Creates new component group entry if one doesn't exist. Otherwise, modifies the existing one
    //
    ComponentGroup<T, BUCKET_SIZE>* cg = &(componentList->lookup[e.id]);
    cg->entity = e;
    cg->bucketArray = &componentList->components;

    for (uint32 i = 0; i < numComponents; i++)
    {
        BucketLocator locator = componentList->components.occupyEmptySlot();
        T* component = componentList->components.addressOf(locator);
        component->entity = e;

        assert(cg->numComponents < MAX_NUM_OF_SAME_COMPONENTS_PER_ENTITY);

        cg->components[cg->numComponents] = locator;
        cg->numComponents++;
    }

    cg->bucketArray = &componentList->components;

    return *cg;
}

template<class T, uint32 BUCKET_SIZE>
ComponentGroup<T, BUCKET_SIZE> getComponents(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
{
    assert(T::multipleAllowedPerEntity);
    
    ComponentGroup<T, BUCKET_SIZE> result;
    
    auto it = componentList->lookup.find(e.id);
    if (it != componentList->lookup.end())
    {
        result = it->second;
    }

    return result;
}

template<class T, uint32 BUCKET_SIZE>
bool removeComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, T* toRemove)
{
    auto it = componentList->lookup.find(toRemove->entity.id);
    if (it == componentList->lookup.end())
    {
        return false;
    }

    ComponentGroup<T, BUCKET_SIZE> &thisEntitiesComponents = it->second;

    for (uint32 i = 0; i < thisEntitiesComponents.numComponents; i++)
    {
        T* component = &thisEntitiesComponents[i];

        if (component == toRemove)
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

            return true;
        }
    }

    return false;
}

Entity makeEntity(Ecs* ecs, string16 friendlyName, EntityFlags flags)
{
    Entity result;
    result.id = Ecs::nextEntityId;
    result.ecs = ecs;

    ecs->entities.push_back(result);

    EntityDetails* details = addComponent<EntityDetails>(result);
    assert(details != nullptr);
    
    details->entity.id = result.id;
    details->entity.ecs = result.ecs;
    details->flags = flags;
    details->parent.id = 0;
    details->friendlyName = friendlyName;

    TransformComponent* xfm = addComponent<TransformComponent>(result);
    assert(xfm != nullptr);
    
    xfm->entity.id = result.id;
    xfm->entity.ecs = result.ecs;
    
    Ecs::nextEntityId++;
    return result;
}

bool markEntityForDeletion(Entity e)
{
    EntityDetails* details = getEntityDetails(e);
    if (details == nullptr)
    {
        assert(false);
        return false;
    }

    details->flags |= EntityFlag_MarkedForDeletion;

    Game* game = getGame(e);
    PotentiallyStaleEntity toDelete = PotentiallyStaleEntity(e);
    game->entitiesMarkedForDeletion.push_back(toDelete);

    for (PotentiallyStaleEntity child : *getChildren(e))
    {
        bool childDelete = markEntityForDeletion(getEntity(game, &child));
        assert(childDelete);
    }
    
    return true;
}

EntityDetails* getEntityDetails(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<EntityDetails>(e);
}

bool removeEntityDetails(EntityDetails** ppComponent)
{
    if (!ppComponent) return false;
    if (((*ppComponent)->flags & EntityFlag_MarkedForDeletion) == 0)
    {
        assert(false);
        return false;
    }
    
    bool success = removeComponent(&((*ppComponent)->entity.ecs->entityDetails), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

TransformComponent* getTransformComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<TransformComponent>(e);
}

bool removeTransformComponent(TransformComponent** ppComponent)
{
    if (!ppComponent) return false;
    EntityDetails* details = getEntityDetails((*ppComponent)->entity);
    
    if ((details->flags & EntityFlag_MarkedForDeletion) == 0)
    {
        assert(false);
        return false;
    }
    
    bool success = removeComponent(&((*ppComponent)->entity.ecs->transforms), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

CameraComponent* addCameraComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<CameraComponent>(e);
}

CameraComponent* getCameraComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<CameraComponent>(e);
}

bool removeCameraComponent(CameraComponent** ppComponent)
{
    if (!ppComponent) return false;
    assert((*ppComponent)->entity.id != (*ppComponent)->entity.ecs->scene->game->activeCamera.id);
    bool success = removeComponent(&((*ppComponent)->entity.ecs->cameras), *ppComponent);
    if (success) *ppComponent = nullptr;
    return success;
}

TerrainComponent* addTerrainComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<TerrainComponent>(e);
}

TerrainComponent* getTerrainComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<TerrainComponent>(e);
}

bool removeTerrainComponent(TerrainComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->terrains), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

AgentComponent* addAgentComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<AgentComponent>(e);
}

AgentComponent* getAgentComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<AgentComponent>(e);
}

bool removeAgentComponent(AgentComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->agentComponents), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

WalkComponent* addWalkComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<WalkComponent>(e);
}

WalkComponent* getWalkComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<WalkComponent>(e);
}

bool removeWalkComponent(WalkComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->walkComponents), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

PortalComponent* addPortalComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<PortalComponent>(e);
}

PortalComponent* getPortalComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<PortalComponent>(e);
}

bool removePortalComponent(PortalComponent** ppComponent)
{
    if (!ppComponent) return false;

    PotentiallyStaleEntity pse = (*ppComponent)->connectedPortal;
    PortalComponent* connectedPortal = getPortalComponent(getEntity(getGame((*ppComponent)->entity), &(*ppComponent)->connectedPortal));
    if (connectedPortal != nullptr)
    {
        connectedPortal->connectedPortal.id = 0;
    }
    
    bool success = removeComponent(&((*ppComponent)->entity.ecs->portals), *ppComponent);
    if (success) *ppComponent = nullptr;
    
    return success;
}

DirectionalLightComponent* addDirectionalLightComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<DirectionalLightComponent>(e);
}

DirectionalLightComponent* getDirectionalLightComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<DirectionalLightComponent>(e);
}

ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE> addDirectionalLightComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE>();
    return addComponents(&e.ecs->directionalLights, e, numComponents);
}

ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE> getDirectionalLightComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<DirectionalLightComponent, Ecs::DIRECTIONAL_LIGHT_BUCKET_SIZE>();
    return getComponents(&e.ecs->directionalLights, e);
}

bool removeDirectionalLightComponent(DirectionalLightComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->directionalLights), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

PointLightComponent* addPointLightComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<PointLightComponent>(e);
}

PointLightComponent* getPointLightComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<PointLightComponent>(e);
}

ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> addPointLightComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE>();
    return addComponents(&e.ecs->pointLights, e, numComponents);
}

ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE> getPointLightComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<PointLightComponent, Ecs::POINT_LIGHT_BUCKET_SIZE>();
    return getComponents(&e.ecs->pointLights, e);
}

bool removePointLightComponent(PointLightComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->pointLights), *ppComponent);
    if (success) *ppComponent = nullptr;
    return success;
}

RenderComponent* addRenderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<RenderComponent>(e);
}

RenderComponent* getRenderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<RenderComponent>(e);
}

ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> addRenderComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE>();
    return addComponents(&e.ecs->renderComponents, e, numComponents);
}

ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE> getRenderComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<RenderComponent, Ecs::RENDER_COMPONENT_BUCKET_SIZE>();
    return getComponents(&e.ecs->renderComponents, e);
}

bool removeRenderComponent(RenderComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->renderComponents), *ppComponent);
    if (success) *ppComponent = nullptr;
    return success;
}

ColliderComponent* addColliderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<ColliderComponent>(e);
}

ColliderComponent* getColliderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<ColliderComponent>(e);
}

ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> addColliderComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE>();
    return addComponents(&e.ecs->colliders, e, numComponents);
}

ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE> getColliderComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<ColliderComponent, Ecs::COLLIDER_BUCKET_SIZE>();
    return getComponents(&e.ecs->colliders, e);
}

bool removeColliderComponent(ColliderComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->colliders), *ppComponent);
    if (success) *ppComponent = nullptr;
    return success;
}


ConvexHullColliderComponent* addConvexHullColliderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent<ConvexHullColliderComponent>(e);
}

ConvexHullColliderComponent* getConvexHullColliderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent<ConvexHullColliderComponent>(e);
}

ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE> addConvexHullColliderComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE>();
    return addComponents(&e.ecs->convexHullColliders, e, numComponents);
}

ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE> getConvexHullColliderComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<ConvexHullColliderComponent, Ecs::CONVEX_HULL_COLLIDER_BUCKET_SIZE>();
    return getComponents(&e.ecs->convexHullColliders, e);
}

bool removeConvexHullColliderComponent(ConvexHullColliderComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->convexHullColliders), *ppComponent);
    if (success) *ppComponent = nullptr;
    return success;
}


RaycastResult castRay(Ecs* ecs, Ray ray)
{
    RaycastResult result;
    result.hit = false;
    result.t = -1;

    for (Entity& e : ecs->entities)
    {
#if 1
        EntityDetails* debug_entity_name = getEntityDetails(e);
#endif
        TransformComponent* xfm = getTransformComponent(e);
        
        auto rcs = getRenderComponents(e);
        auto colliders = getColliderComponents(e);
        auto convexColliders = getConvexHullColliderComponents(e);

        // @Slow: maybe cache this on the entity somehow?
        if (rcs.numComponents > 0 || colliders.numComponents > 0 || convexColliders.numComponents > 0)
        {
            Aabb bounds = aabbFromMinMax(Vec3(FLT_MAX), Vec3(-FLT_MAX));

            if (rcs.numComponents > 0)
            {
                Aabb renderAabb = aabbFromRenderComponents(rcs);

                Vec3 minPoint = componentwiseMin(renderAabb.minPoint(), bounds.minPoint());
                Vec3 maxPoint = componentwiseMax(renderAabb.maxPoint(), bounds.maxPoint());

                bounds = aabbFromMinMax(minPoint, maxPoint);
            }

            if (colliders.numComponents > 0)
            {
                Aabb colliderAabb = aabbFromColliders(colliders);

                Vec3 minPoint = componentwiseMin(colliderAabb.minPoint(), bounds.minPoint());
                Vec3 maxPoint = componentwiseMax(colliderAabb.maxPoint(), bounds.maxPoint());

                bounds = aabbFromMinMax(minPoint, maxPoint);
            }

            if (convexColliders.numComponents > 0)
            {
                Aabb colliderAabb = aabbFromConvexColliders(convexColliders);

                Vec3 minPoint = componentwiseMin(colliderAabb.minPoint(), bounds.minPoint());
                Vec3 maxPoint = componentwiseMax(colliderAabb.maxPoint(), bounds.maxPoint());

                bounds = aabbFromMinMax(minPoint, maxPoint);
            }
    
            float32 t = rayVsAabb(ray, bounds);

            if (t >= 0)
            {
                if (!result.hit || t < result.t)
                {
                    result.hit = true;
                    result.t = t;
                    result.hitEntity = e;
                }
            }
        }
    }

    return result;
}

Ecs::Ecs()
{
    componentListByType[typeid(EntityDetails)] = &entityDetails;
    componentListByType[typeid(TransformComponent)] = &transforms;
    componentListByType[typeid(CameraComponent)] = &cameras;
    componentListByType[typeid(DirectionalLightComponent)] = &directionalLights;
    componentListByType[typeid(PointLightComponent)] = &pointLights;
    componentListByType[typeid(RenderComponent)] = &renderComponents;
    componentListByType[typeid(PortalComponent)] = &portals;
    componentListByType[typeid(ColliderComponent)] = &colliders;
    componentListByType[typeid(ConvexHullColliderComponent)] = &convexHullColliders;
    componentListByType[typeid(AgentComponent)] = &agentComponents;
    componentListByType[typeid(WalkComponent)] = &walkComponents;
}
