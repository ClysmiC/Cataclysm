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
#include "components/TerrainComponent.h"
#include "components/WalkComponent.h"

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

template<class T, uint32 BUCKET_SIZE>
T* addComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
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

template<class T, uint32 BUCKET_SIZE>
T* getComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
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

    EntityDetails* details = addComponent(&ecs->entityDetails, result);
    assert(details != nullptr);
    
    details->entity.id = result.id;
    details->entity.ecs = result.ecs;
    details->flags = flags;
    details->parent.id = 0;
    details->friendlyName = friendlyName;

    TransformComponent* xfm = addComponent(&ecs->transforms, result);
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
    return getComponent(&e.ecs->entityDetails, e);
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
    return getComponent(&e.ecs->transforms, e);
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
    return addComponent(&e.ecs->cameras, e);
}

CameraComponent* getCameraComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->cameras, e);
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
    return addComponent(&e.ecs->terrains, e);
}

TerrainComponent* getTerrainComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->terrains, e);
}

bool removeTerrainComponent(TerrainComponent** ppComponent)
{
    if (!ppComponent) return false;
    bool success = removeComponent(&((*ppComponent)->entity.ecs->terrains), *ppComponent);

    if (success) *ppComponent = nullptr;

    return success;
}

WalkComponent* addWalkComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent(&e.ecs->walkComponents, e);
}

WalkComponent* getWalkComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->walkComponents, e);
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
    return addComponent(&e.ecs->portals, e);
}

PortalComponent* getPortalComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->portals, e);
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
    return addComponent(&e.ecs->directionalLights, e);
}

DirectionalLightComponent* getDirectionalLightComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->directionalLights, e);
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
    return addComponent(&e.ecs->pointLights, e);
}

PointLightComponent* getPointLightComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->pointLights, e);
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
    return addComponent(&e.ecs->renderComponents, e);
}

RenderComponent* getRenderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->renderComponents, e);
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
    return addComponent(&e.ecs->colliders, e);
}

ColliderComponent* getColliderComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->colliders, e);
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



RaycastResult castRay(Ecs* ecs, Ray ray)
{
    RaycastResult result;
    result.hit = false;
    result.t = -1;

    for (Entity& e : ecs->entities)
    {       
        TransformComponent* xfm = getTransformComponent(e);
        
        auto rcs = getRenderComponents(e);
        auto colliders = getColliderComponents(e);

        // @Slow: maybe cache this on the entity somehow?
        if (rcs.numComponents > 0 || colliders.numComponents > 0)
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

