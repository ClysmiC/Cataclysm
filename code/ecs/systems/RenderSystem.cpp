#include "als/als_bucket_array.h"

#include "GL/glew.h"

#include "Quad.h"
#include "DebugDraw.h"
#include "RenderSystem.h"
#include "ecs/Ecs.h"
#include "Scene.h"
#include "ecs/components/CameraComponent.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/PointLightComponent.h"
#include "ecs/components/DirectionalLightComponent.h"
#include "ecs/components/TransformComponent.h"
#include "resource/resources/Shader.h"

#include <string>

PointLightComponent* closestPointLight(Ecs* ecs, ITransform* xfm)
{    
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

void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, ITransform* cameraXfm, bool renderingViaPortal, ITransform* destPortalXfm)
{
    // TODO: better way of picking which directional light to use for shadow mapping?
    bool hasShadowMap = false;
    if (ecs->directionalLights.count() > 0)
    {
        hasShadowMap = true;
        DirectionalLightComponent* dirLight = &ecs->directionalLights[0];

        Vec3 veryFarAwayPoint = -dirLight->direction * 1000;

        // TODO: set up ortho camera and render from veryFarAwayPoint to depth buffer

        
    }
    
    FOR_BUCKET_ARRAY (ecs->renderComponents.components)
    {
        RenderComponent &rc = *it.ptr;
        if (!rc.isVisible) continue;
        
        TransformComponent* xfm = getTransformComponent(rc.entity);

        if (renderingViaPortal)
        {
            bool behindDestPortal = dot(destPortalXfm->forward(), destPortalXfm->position() - xfm->position()) > 0;
            if (behindDestPortal) continue;
        }

        // TODO: Lighting is a total mess... figure out a better way to do it
        if (rc.material->receiveLight)
        {
            PointLightComponent* pl = closestPointLight(ecs, xfm);
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
            else
            {
                setVec3(shader, "pointLights[0].intensity", Vec3(0));
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

            if (ecs->directionalLights.count() == 0)
            {
                string64 intensityVarName = ("directionalLights[" + std::to_string(it.index) + "].intensity").c_str();
                setVec3(shader, intensityVarName, Vec3(0, 0, 0));
            }
        }

        drawRenderComponent(&rc, xfm, camera, cameraXfm);
    }
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
        if (pc->connectedPortal.id == 0) continue;

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
                bool closeEnoughToClipThroughNearPlane = distance(cameraXfm->position(), portalPlane) < .12;
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
                renderScene(getConnectedScene(pc), camera, &portalViewpointXfm, recursionLevel + 1, destSceneXfm);
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
