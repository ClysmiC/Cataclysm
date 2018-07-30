#include "GL/glew.h"

#include "Ecs.h"
#include "Scene.h"
#include "assert.h"

#include "Game.h"

#include "Quad.h"
#include "DebugDraw.h"
#include "DebugGlobal.h"

#include "Shader.h"
#include "Transform.h"

#include "Game.h"
#include <string>

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"

#include "als_util.h"

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

template<class T, uint32 BUCKET_SIZE>
T* addComponent(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
{
    assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist

    BucketLocator location = componentList->components.occupyEmptySlot();
    T* result = componentList->components.addressOf(location);
    result->entity = e;

    ComponentGroup<T, BUCKET_SIZE> cg;
    cg.firstComponent = location;
    cg.numComponents = 1;
    cg.entity = e;
    cg.bucketArray = &componentList->components;

    componentList->lookup[e.id] = cg;
    
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

    return componentList->components.addressOf(it->second.firstComponent);
}

template<class T, uint32 BUCKET_SIZE>
ComponentGroup<T, BUCKET_SIZE> addComponents(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e, uint32 numComponents)
{
    assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist
    
    BucketLocator firstComponent = BucketLocator();

    for (uint32 i = 0; i < numComponents; i++)
    {
        BucketLocator locator = componentList->components.occupyEmptySlot();
        T* result = componentList->components.addressOf(locator);
        result->entity = e;

        if (i == 0)
        {
            firstComponent = locator;
        }
    }

    ComponentGroup<T, BUCKET_SIZE> cg;
    cg.firstComponent = firstComponent;
    cg.numComponents = numComponents;
    cg.entity = e;
    cg.bucketArray = &componentList->components;

    componentList->lookup[e.id] = cg;

    return cg;
}

template<class T, uint32 BUCKET_SIZE>
ComponentGroup<T, BUCKET_SIZE> getComponents(Ecs::ComponentList<T, BUCKET_SIZE>* componentList, Entity e)
{
    ComponentGroup<T, BUCKET_SIZE> result;
    
    auto it = componentList->lookup.find(e.id);
    if (it != componentList->lookup.end())
    {
        result = it->second;
    }

    return result;
}

Entity makeEntity(Ecs* ecs, string16 friendlyName)
{
    Entity result;
    result.id = Ecs::nextEntityId;
    result.ecs = ecs;

    ecs->entityList.push_back(result);

    EntityDetails* details = addComponent(&ecs->entityDetails, result);
    details->entity.id = result.id;
    details->entity.ecs = result.ecs;
    details->flags = 0;
    details->friendlyName = friendlyName;
    
    Ecs::nextEntityId++;
    return result;
}

EntityDetails* getEntityDetails(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->entityDetails, e);
}

TransformComponent* addTransformComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent(&e.ecs->transforms, e);
}

TransformComponent* getTransformComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->transforms, e);
}

CameraComponent* addCameraComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return addComponent(&e.ecs->cameras, e);
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

CameraComponent* getCameraComponent(Entity e)
{
    if (e.id == 0) return nullptr;
    return getComponent(&e.ecs->cameras, e);
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

void renderContentsOfAllPortals(Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel)
{
    if (recursionLevel > 0)
    {
        // TODO: handle recursive renders (i.e., looking through a portal through another portal)
        return;
    }

    FOR_BUCKET_ARRAY(scene->ecs.portals.components)
    {
        //
        // Calculate the position and orientation of the camera sitting in the dest scene and looking "through" the portal
        // into the dest scene.
        //
        PortalComponent* pc = scene->ecs.portals.components.addressOf(it.locator);

        TransformComponent* sourceSceneXfm = getTransformComponent(pc->entity);
        TransformComponent* destSceneXfm = getConnectedSceneXfm(pc);

        Transform portalViewpointXfm(cameraXfm->position(), cameraXfm->orientation());
        rebaseTransformInPlace(pc, &portalViewpointXfm);

        glEnable(GL_STENCIL_TEST);
        {
            //
            // Render the portal in the source scene and write to stencil buffer
            //
            glDepthMask(GL_FALSE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            {
                
                //
                // Render to stencil
                //
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glStencilMask(0xFF);
            
                Shader* shader = portalShader();
                uint32 portalVao = quadVao();

                Transform perturbedPortalXfm;
                perturbedPortalXfm.setPosition(sourceSceneXfm->position());
                perturbedPortalXfm.setOrientation(sourceSceneXfm->orientation());
                perturbedPortalXfm.setScale(sourceSceneXfm->scale());

                Plane portalPlane = Plane(sourceSceneXfm->position(), outOfPortalNormal(pc));
                bool closeEnoughToClipThroughNearPlane = distanceSquared(cameraXfm->position(), portalPlane) < .11 * .11;
                if (closeEnoughToClipThroughNearPlane)
                {
                    perturbedPortalXfm.setPosition(perturbedPortalXfm.position() + (intoPortalNormal(pc) * .1));
                }

                Mat4 model = sourceSceneXfm->matrix();
                Mat4 view = worldToView(cameraXfm);
                Mat4 projection = camera->projectionMatrix;

                bind(shader);
                setMat4(shader, "model", model);
                setMat4(shader, "view", view);
                setMat4(shader, "projection", projection);
                setVec3(shader, "debugColor", Vec3(0, 1, 0));

                glBindVertexArray(portalVao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);

            }
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            //
            // Render from portal camera
            //
            {
                glStencilFunc(GL_EQUAL, 1, 0xFF);
                glStencilMask(0x00);
        
                if (!debug_hidePortalContents)
                {
                    renderScene(getConnectedScene(pc), camera, &portalViewpointXfm, recursionLevel + 1, destSceneXfm);
                }
            }
        }
        glDisable(GL_STENCIL_TEST);

            
        //
        // Render the portal's depth to the depth buffer
        //
        glDepthFunc(GL_ALWAYS);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        {
            Shader* shader = portalShader();
            uint32 portalVao = quadVao();

            Mat4 model = sourceSceneXfm->matrix();
            Mat4 view = worldToView(cameraXfm);
            Mat4 projection = camera->projectionMatrix;

            bind(shader);
            setMat4(shader, "model", model);
            setMat4(shader, "view", view);
            setMat4(shader, "projection", projection);
            setVec3(shader, "debugColor", Vec3(0, 1, 0));

            glBindVertexArray(portalVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
        glDepthFunc(GL_LESS);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);

        DebugDraw::instance().drawRect3(sourceSceneXfm->position(), Vec3(getDimensions(pc), 0.2), sourceSceneXfm->orientation());
    }
}

void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, ITransform* cameraXfm, bool renderingViaPortal, ITransform* destPortalXfm)
{
    FOR_BUCKET_ARRAY (ecs->renderComponents.components)
    {
        RenderComponent &rc = *it.ptr;
        TransformComponent* xfm = getTransformComponent(rc.entity);

        if (renderingViaPortal)
        {
            bool behindDestPortal = dot(destPortalXfm->forward(), destPortalXfm->position() - xfm->position()) > 0;
            if (behindDestPortal) continue;
        }

        assert(xfm != nullptr); // Render component cannot exist without corresponding transform component
        if (xfm == nullptr) continue;

        // TODO: pass closest lights to RenderComponent draw...
        // ECS should be responsible for finding the light, but render component should be responsible
        // for handling the lighting
        if (rc.material->receiveLight)
        {
            PointLightComponent* pl = closestPointLight(xfm);
            Shader* shader = rc.material->shader;
            bind(shader);

            if (pl != nullptr)
            {
                TransformComponent* plXfm = getTransformComponent(pl->entity);
            
                setVec3(shader, "pointLights[0].posWorld", plXfm->position());
                setVec3(shader, "pointLights[0].intensity", pl->intensity);
                setFloat(shader, "pointLights[0].attenuationConstant", pl->attenuationConstant);
                setFloat(shader, "pointLights[0].attenuationLinear", pl->attenuationLinear);
                setFloat(shader, "pointLights[0].attenuationQuadratic", pl->attenuationQuadratic);
            }

            FOR_BUCKET_ARRAY (ecs->directionalLights.components)
            {
                // TODO: what happens if the number of directional lights exceeds the number allowed in the shader?
                // How can we guarantee it doesnt? Should we just hard code a limit that is the same as the limit
                // in the shader? Is that robust when we change the shader?
                DirectionalLightComponent* dlc = it.ptr;

                string64 directionVarName = ("directionalLights[" + std::to_string(it.index) + "].direction").c_str();
                string64 intensityVarName = ("directionalLights[" + std::to_string(it.index) + "].intensity").c_str();
                
                setVec3(shader, directionVarName, dlc->direction);
                setVec3(shader, intensityVarName, dlc->intensity);
            }
        }

        drawRenderComponent(&rc, xfm, camera, cameraXfm);
    }
}

RaycastResult castRay(Ecs* ecs, Ray ray)
{
    RaycastResult result;
    result.hit = false;
    result.t = -1;

    for (Entity& e : ecs->entityList)
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

PointLightComponent* closestPointLight(TransformComponent* xfm)
{
    Ecs* ecs = xfm->entity.ecs;
    
    PointLightComponent* closest = nullptr;
    float32 closestDistance = FLT_MAX;

    FOR_BUCKET_ARRAY (ecs->pointLights.components)
    {
        PointLightComponent* pl = it.ptr;
        TransformComponent* plXfm = getTransformComponent(pl->entity);

        assert(plXfm != nullptr);
        if (plXfm == nullptr) continue;
        
        float32 dist = distance(xfm->position(), plXfm->position());

        if (dist < closestDistance)
        {
            closestDistance = dist;
            closest = pl;
        }
    }

    return closest;
}

void walkAndCamera(Game* game)
{
    TransformComponent* xfm = getTransformComponent(game->player);
    TransformComponent* cameraXfm = getTransformComponent(game->activeCamera);
    WalkComponent* walk = getWalkComponent(game->player);
    ColliderComponent* collider = getColliderComponent(game->player);
    
    TerrainComponent *terrain = getTerrainComponent(walk->terrain);
    
    Vec3 posBeforeMove = xfm->position();
    
    Plane movementPlane(Vec3(0, 0, 0), Vec3(0, 1, 0));
    
    float32 cameraTurnSpeed = 1.5; // Deg / pixel / Sec
    float32 cameraSpeed = 5;
    float32 deltaTS = deltaTMs / 1000.0f;

    if (keys[GLFW_KEY_LEFT_SHIFT])
    {
        cameraSpeed *= 2;
    }

    Vec3 moveRight   = normalize( project(xfm->right(), movementPlane) );
    Vec3 moveLeft    = normalize( -moveRight );
    Vec3 moveForward = normalize( project(xfm->forward(), movementPlane) );
    Vec3 moveBack    = normalize( -moveForward );

    // Uncomment this (and the asserts) to follow the pitch of the camera when
    // moving forward or backward.
    // moveForward = normalize(xfm->forward());

    assert(FLOAT_EQ(moveRight.y, 0, EPSILON));
    assert(FLOAT_EQ(moveLeft.y, 0, EPSILON));
    assert(FLOAT_EQ(moveForward.y, 0, EPSILON));
    assert(FLOAT_EQ(moveBack.y, 0, EPSILON));

    bool draggingCameraInEditMode =
        game->editor.isEnabled &&
        !game->editor.translator.isHandleSelected &&
        mouseButtons[GLFW_MOUSE_BUTTON_1] &&
        !ImGui::GetIO().WantCaptureMouse;

    if (!game->editor.isEnabled || draggingCameraInEditMode)
    {
        if (mouseXPrev != FLT_MAX && mouseYPrev != FLT_MAX)
        {
            // Rotate
            float32 deltaMouseX = mouseX - mouseXPrev;
            float32 deltaMouseY = mouseY - mouseYPrev;

            if (draggingCameraInEditMode)
            {
                // Drag gesture moves in opposite direction
                deltaMouseX = -deltaMouseX;
                deltaMouseY = -deltaMouseY;
            }

            Quaternion deltaYaw;
            Quaternion deltaPitch;
            deltaYaw = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * -deltaMouseX * deltaTS); // yaw
            deltaPitch = axisAngle(Vec3(1, 0, 0), cameraTurnSpeed * deltaMouseY * deltaTS); // pitch

            // "Player" just yaws.
            // Camera yaws and pitches
            xfm->setOrientation(deltaYaw * xfm->orientation());

            cameraXfm->setLocalOrientation(deltaPitch * cameraXfm->localOrientation());
        }

        if (keys[GLFW_KEY_W])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveForward * cameraSpeed * deltaTS);
        }
        else if (keys[GLFW_KEY_S])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveBack * cameraSpeed * deltaTS);
        }
        
        if (keys[GLFW_KEY_A])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveLeft * cameraSpeed * deltaTS);
        }
        else if (keys[GLFW_KEY_D])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveRight * cameraSpeed * deltaTS);
        }
    }

    if (terrain)
    {
        float32 height = getTerrainHeight(terrain, xfm->position().x, xfm->position().z);

        xfm->setPosition(
            Vec3(
                xfm->position().x,
                height,
                xfm->position().z
             )
        );
    }

    //
    // Go thru portal
    //
    FOR_BUCKET_ARRAY (game->activeScene->ecs.portals.components)
    {
        PortalComponent* pc = it.ptr;
        ColliderComponent* cc = getColliderComponent(pc->entity);

        if (pointInsideCollider(cc, xfm->position()))
        {
            Vec3 portalPos = getTransformComponent(pc->entity)->position();
            Vec3 portalToOldPos = posBeforeMove - portalPos;
            Vec3 portalToPos = xfm->position() - portalPos;
            
            if (dot(portalToOldPos, outOfPortalNormal(pc)) >= 0)
            {
                rebaseTransformInPlace(pc, xfm);
                game->activeScene = pc->connectedPortal->entity.ecs->scene;
            }
        }
    }
}
