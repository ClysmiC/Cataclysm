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

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

template<class T>
T* addComponent(Ecs::ComponentList<T>* componentList, Entity e)
{
    assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist
    assert(componentList->size < COMPONENT_ARRAY_SIZE);

    T componentToAdd;
    componentToAdd.entity = e;
    
    componentList->components[componentList->size] = componentToAdd;
    T* result = &(componentList->components[componentList->size]);

    componentList->size++;

    ComponentGroup<T> cg;
    cg.components = result;
    cg.numComponents = 1;
    componentList->lookup[e.id] = cg;
    
    return result;    
}

template<class T>
T* getComponent(Ecs::ComponentList<T>* componentList, Entity e)
{
    auto it = componentList->lookup.find(e.id);
    if (it == componentList->lookup.end())
    {
        return nullptr;
    }

    return it->second.components;
}

template<class T>
ComponentGroup<T> addComponents(Ecs::ComponentList<T>* componentList, Entity e, uint32 numComponents)
{
    assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist
    
    T* firstComponent = nullptr;

    for (uint32 i = 0; i < numComponents; i++)
    {
        assert(componentList->size < COMPONENT_ARRAY_SIZE);
        
        T component;
        component.entity = e;
        
        componentList->components[componentList->size] = component;
        if (i == 0)
        {
            firstComponent = &(componentList->components[componentList->size]);
        }
        
        componentList->size++;
    }

    ComponentGroup<T> cg;
    cg.entity = e;
    cg.components = firstComponent;
    cg.numComponents = numComponents;
    componentList->lookup[e.id] = cg;

    return cg;
}

template<class T>
ComponentGroup<T> getComponents(Ecs::ComponentList<T>* componentList, Entity e)
{
    auto it = componentList->lookup.find(e.id);
    if (it == componentList->lookup.end())
    {
        ComponentGroup<T> result;
        result.components = nullptr;
        result.numComponents = 0;
        return result;
    }

    return it->second;
}

Entity makeEntity(Ecs* ecs, string16 friendlyName)
{
    Entity result;
    result.id = Ecs::nextEntityId;
    result.flags = 0;
    result.ecs = ecs;
    result.friendlyName = friendlyName;

    ecs->entities.push_back(result);
    
    Ecs::nextEntityId++;
    return result;
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

ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<PointLightComponent>();
    return addComponents(&e.ecs->pointLights, e, numComponents);
}

ComponentGroup<PointLightComponent> getPointLightComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<PointLightComponent>();
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

ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<RenderComponent>();
    return addComponents(&e.ecs->renderComponents, e, numComponents);
}

ComponentGroup<RenderComponent> getRenderComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<RenderComponent>();
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

ComponentGroup<ColliderComponent> addColliderComponents(Entity e, uint32 numComponents)
{
    if (e.id == 0) return ComponentGroup<ColliderComponent>();
    return addComponents(&e.ecs->colliders, e, numComponents);
}

ComponentGroup<ColliderComponent> getColliderComponents(Entity e)
{
    if (e.id == 0) return ComponentGroup<ColliderComponent>();
    return getComponents(&e.ecs->colliders, e);
}

void renderContentsOfAllPortals(Scene* scene, CameraComponent* camera, Transform* cameraXfm, uint32 recursionLevel)
{
    if (recursionLevel > 0)
    {
        // TODO: handle recursive renders (i.e., looking through a portal through another portal)
        return;
    }

    for (uint32 i = 0; i < scene->ecs.portals.size; i++)
    {
        //
        // Calculate the position and orientation of the camera sitting in the dest scene and looking "through" the portal
        // into the dest scene.
        //
        PortalComponent* pc = &scene->ecs.portals.components[i];

        Transform* sourceSceneXfm = getTransformComponent(pc->entity);
        Transform* destSceneXfm = getConnectedSceneXfm(pc);

        Transform portalViewpointXfm(cameraXfm->position, cameraXfm->orientation);
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
                perturbedPortalXfm.position = sourceSceneXfm->position;
                perturbedPortalXfm.orientation = sourceSceneXfm->orientation;
                perturbedPortalXfm.scale = sourceSceneXfm->scale;

                Plane portalPlane = Plane(sourceSceneXfm->position, outOfPortalNormal(pc));
                bool closeEnoughToClipThroughNearPlane = distanceSquared(cameraXfm->position, portalPlane) < .11 * .11;
                if (closeEnoughToClipThroughNearPlane)
                {
                    perturbedPortalXfm.position += (intoPortalNormal(pc) * .1);
                }

                Mat4 model = modelToWorld(sourceSceneXfm);
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

            Mat4 model = modelToWorld(sourceSceneXfm);
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

        DebugDraw::instance().drawRect3(sourceSceneXfm->position, Vec3(getDimensions(pc), 0.2), sourceSceneXfm->orientation);
    }
}

void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, Transform* cameraXfm, bool renderingViaPortal, Transform* destPortalXfm)
{
    for (uint32 i = 0; i < ecs->renderComponents.size; i++)
    {
        RenderComponent &rc = ecs->renderComponents.components[i];
        TransformComponent* xfm = getTransformComponent(rc.entity);

        if (renderingViaPortal)
        {
            bool behindDestPortal = dot(destPortalXfm->forward(), destPortalXfm->position - xfm->position) > 0;
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
            
                setVec3(shader, "pointLights[0].posWorld", plXfm->position);
                setVec3(shader, "pointLights[0].intensity", pl->intensity);
                setFloat(shader, "pointLights[0].attenuationConstant", pl->attenuationConstant);
                setFloat(shader, "pointLights[0].attenuationLinear", pl->attenuationLinear);
                setFloat(shader, "pointLights[0].attenuationQuadratic", pl->attenuationQuadratic);
            }

            for (uint32 j = 0; j < ecs->directionalLights.size; j++)
            {
                // TODO: what happens if the number of directional lights exceeds the number allowed in the shader?
                // How can we guarantee it doesnt? Should we just hard code a limit that is the same as the limit
                // in the shader? Is that robust when we change the shader?
                DirectionalLightComponent* dlc = ecs->directionalLights.components + j;

                string64 directionVarName = ("directionalLights[" + std::to_string(j) + "].direction").c_str();
                string64 intensityVarName = ("directionalLights[" + std::to_string(j) + "].intensity").c_str();
                
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

    for (Entity& e : ecs->entities)
    {
        TransformComponent* xfm = getTransformComponent(e);
        RenderComponent* rc = getRenderComponent(e);
    
        if (rc != nullptr && xfm != nullptr)
        {
            Aabb bounds = transformedAabb(rc->submesh->mesh->bounds, xfm);
            float32 t = rayAabbTest(ray, bounds);

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

    for (uint32 i = 0; i < ecs->pointLights.size; i++)
    {
        PointLightComponent* pl = &(ecs->pointLights.components[i]);
        TransformComponent* plXfm = getTransformComponent(pl->entity);

        assert(plXfm != nullptr);
        if (plXfm == nullptr) continue;
        
        float32 dist = distance(xfm->position, plXfm->position);

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
    // @Hack, we are updating camera here too
    TransformComponent* xfm = getTransformComponent(game->activeCamera);
    Vec3 posBeforeMove = xfm->position;
    
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

            Quaternion deltaYawAndPitch;
            deltaYawAndPitch = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * -deltaMouseX * deltaTS); // yaw
            deltaYawAndPitch = deltaYawAndPitch * axisAngle(moveRight, cameraTurnSpeed * deltaMouseY * deltaTS); // pitch

            xfm->orientation = deltaYawAndPitch * xfm->orientation;

            float camRightY = xfm->right().y;
            assert(FLOAT_EQ(camRightY, 0, EPSILON));
        }

        if (keys[GLFW_KEY_W])
        {
            xfm->position += moveForward * cameraSpeed * deltaTS;
        }
        else if (keys[GLFW_KEY_S])
        {
            xfm->position += moveBack * cameraSpeed * deltaTS;
        }
        
        if (keys[GLFW_KEY_A])
        {
            xfm->position += moveLeft * cameraSpeed * deltaTS;
        }
        else if (keys[GLFW_KEY_D])
        {
            xfm->position += moveRight * cameraSpeed * deltaTS;
        }
    }

    for (uint32 i = 0; i < game->activeScene->ecs.portals.size; i++)
    {
        PortalComponent* pc = &game->activeScene->ecs.portals.components[i];
        ColliderComponent* cc = getColliderComponent(pc->entity);

        if (pointInsideCollider(cc, xfm->position))
        {
            Vec3 portalPos = getTransformComponent(pc->entity)->position;
            Vec3 portalToOldPos = posBeforeMove - portalPos;
            Vec3 portalToPos = xfm->position - portalPos;
            
            if (dot(portalToOldPos, outOfPortalNormal(pc)) >= 0)
            {
                if(dot(portalToPos, intoPortalNormal(pc)) < 0)
                {
                    __debugbreak();
                }

                rebaseTransformInPlace(pc, xfm);
                game->activeScene = pc->connectedPortal->entity.ecs->scene;
            }
        }
    }
    
    // move x and z according to inputs

    // if above a terrain component, set height to the terrain component height
}
