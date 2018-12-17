#include "Scene.h"
#include "resource/resources/Cubemap.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/CameraComponent.h"
#include "ecs/components/PointLightComponent.h"
#include "ecs/components/DirectionalLightComponent.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/ConvexHullColliderComponent.h"
#include "ecs/components/ColliderComponent.h"
#include "ecs/components/TerrainComponent.h"
#include "ecs/components/AgentComponent.h"
#include "ecs/components/WalkComponent.h"

#include "ecs/systems/RenderSystem.h"

Scene::Scene()
{
    this->ecs.scene = this;
    this->cubemap = nullptr;
}

void addCubemap(Scene* scene, Cubemap* cubemap)
{
    scene->cubemap = cubemap;
}

void renderScene(Renderer* renderer, Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel, ITransform* destPortalXfm)
{
    if (recursionLevel > 3) return;
    
    if (scene->cubemap != nullptr) renderCubemap(scene->cubemap, camera, cameraXfm);

    renderContentsOfAllPortals(renderer, scene, camera, cameraXfm, recursionLevel);
    renderAllRenderComponents(renderer, &scene->ecs, camera, cameraXfm, recursionLevel > 0, destPortalXfm);
}




RaycastResult castRay(Scene* scene, Ray ray)
{
    Ecs* ecs = &scene->ecs;

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
