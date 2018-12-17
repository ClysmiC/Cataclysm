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

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

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
    EntityDetails* details = getComponent<EntityDetails>(e);
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


RaycastResult castRay(Ecs* ecs, Ray ray)
{
    RaycastResult result;
    result.hit = false;
    result.t = -1;

    for (Entity& e : ecs->entities)
    {
#if 1
        EntityDetails* debug_entity_name = getComponent<EntityDetails>(e);
#endif
        TransformComponent* xfm = getComponent<TransformComponent>(e);
        
        auto rcs = getComponents<RenderComponent>(e);
        auto colliders = getComponents<ColliderComponent>(e);
        auto convexColliders = getComponents<ConvexHullColliderComponent>(e);

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
