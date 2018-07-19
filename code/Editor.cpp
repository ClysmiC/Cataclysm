#include "Editor.h"
#include "Game.h"
#include "Reflection.h"

#include "imgui/imgui.h"
#include "GLFW/glfw3.h"

#include "DebugDraw.h"

EditorState::EditorState()
{
    this->translator.pseudoEntity = makeEntity(&this->pseudoEcs, "Translator");
    addTransformComponent(this->translator.pseudoEntity);
    auto colliders = addColliderComponents(this->translator.pseudoEntity, 3);

    this->translator.xAxisHandle = &colliders[0];
    this->translator.yAxisHandle = &colliders[1];
    this->translator.zAxisHandle = &colliders[2];

    float32 longDim = 2;
    float32 shortDim = 0.25;

    this->translator.xAxisHandle->type = ColliderType::RECT3;
    this->translator.xAxisHandle->xfmOffset = Vec3(longDim / 2, 0, 0);
    this->translator.xAxisHandle->rect3Lengths = Vec3(longDim, shortDim, shortDim);

    this->translator.yAxisHandle->type = ColliderType::RECT3;
    this->translator.yAxisHandle->xfmOffset = Vec3(0, longDim / 2, 0);
    this->translator.yAxisHandle->rect3Lengths = Vec3(shortDim, longDim, shortDim);

    this->translator.zAxisHandle->type = ColliderType::RECT3;
    this->translator.zAxisHandle->xfmOffset = Vec3(0, 0, longDim / 2);
    this->translator.zAxisHandle->rect3Lengths = Vec3(shortDim, shortDim, longDim);
}

void showEditor(EditorState* editor)
{
    //
    // Preamble
    //
    Game* game = editor->game;
    
    // Ignores clicks on ImGui
    bool mouseHeld = mouseButtons[GLFW_MOUSE_BUTTON_1] && !ImGui::GetIO().WantCaptureMouse;
    bool mousePressed = mouseHeld && !lastMouseButtons[GLFW_MOUSE_BUTTON_1];
    bool clickHandled = false;

    if (mouseHeld == true)
    {
        int x = 0;
    }

    Ray prevRayThruScreen = rayThroughScreenCoordinate(getCameraComponent(game->activeCamera), Vec2(mouseXPrev, mouseYPrev));
    Ray rayThruScreen = rayThroughScreenCoordinate(getCameraComponent(game->activeCamera), Vec2(mouseX, mouseY));

    //
    // Draw aabb and collider
    //
    {
        Entity e = editor->selectedEntity;

        if (e.id != 0)
        {
            if (game->editor.drawAabb)
            {
                DebugDraw::instance().drawAabb(e);
            }
                
            if (game->editor.drawCollider)
            {
                ColliderComponent* cc = getColliderComponent(e);

                if (cc) DebugDraw::instance().drawCollider(cc);
            }
        }
    }

    //
    // Scale and draw 3d translator handles
    //
    {
        TransformComponent* tc = getTransformComponent(editor->selectedEntity);

        if (tc)
        {
            TransformComponent* toolXfm = getTransformComponent(editor->translator.pseudoEntity);
            toolXfm->position = tc->position;

            if (editor->isLocalXfm)
            {
                toolXfm->orientation = tc->orientation;
            }
            
            TransformComponent* cameraXfm = getTransformComponent(game->activeCamera);

            const float32 SCALE_FACTOR = 0.1;

            float32 lengthFromCamera = length(tc->position - cameraXfm->position);
            float32 scale = lengthFromCamera * SCALE_FACTOR;

            toolXfm->scale = Vec3(scale);

            float32 arrowLength = editor->translator.xAxisHandle->rect3Lengths.x;
            arrowLength *= scale;
            
            Vec3 oldColor = DebugDraw::instance().color;

            glClear(GL_DEPTH_BUFFER_BIT);
            
            DebugDraw::instance().color = Vec3(1, 0, 0);
            DebugDraw::instance().drawArrow(tc->position, tc->position + arrowLength * toolXfm->right());

            DebugDraw::instance().color = Vec3(0, 1, 0);
            DebugDraw::instance().drawArrow(tc->position, tc->position + arrowLength * toolXfm->up());

            DebugDraw::instance().color = Vec3(0, 0, 1);
            DebugDraw::instance().drawArrow(tc->position, tc->position + arrowLength * toolXfm->back());

            DebugDraw::instance().color = oldColor;

            //
            // Handle translator select 
            //
            if (mousePressed && !clickHandled)
            {
                editor->translator.isHandleSelected = false;
                float32 minimumHit = FLT_MAX;
                
                // raycast against x, y, and z colliders
                for (uint32 i = 0; i < 3; i++)
                {
                    DebugDraw::instance().drawCollider(editor->translator.handles[i]);
                    float32 t = rayVsCollider(rayThruScreen, editor->translator.handles[i]);
                    if (t >= 0)
                    {
                        if (t < minimumHit)
                        {
                            minimumHit = t;
                            editor->translator.selectedHandle = (Axis3D)i;
                            editor->translator.isHandleSelected = true;
                            clickHandled = true;
                        }
                    }
                }
            }
            
            //
            // Handle translator drag
            //
            if (editor->translator.isHandleSelected && mouseHeld)
            {
                // TODO: case where drag axis is almost parallel to the camera,
                //       you can accidentally drag the entity VERY far with a small
                //       mouse movement. Consider setting a max limit to the drag
                //       per frame (should depend on distance from camera)
                
                Vec3 planePoint1 = tc->position;
                Vec3 planePoint2 = tc->position + toolXfm->orientation * Vec3(editor->translator.selectedHandle);

                Vec3 dragAxis = (planePoint2 - planePoint1).normalizeInPlace();
                Vec3 toCamera = cameraXfm->position - tc->position;

                Vec3 planeNormal = cross(dragAxis, cross(toCamera, dragAxis)).normalizeInPlace();

                Plane dragPlane = Plane(planePoint1, planeNormal);

                float32 tPrev = rayVsPlaneOneSided(prevRayThruScreen, dragPlane);
                float32 t = rayVsPlaneOneSided(rayThruScreen, dragPlane);

                if (t >= 0 && tPrev >= 0)
                {
                    Vec3 prevIntersectionPoint = pointOnRay(prevRayThruScreen, tPrev);
                    Vec3 intersectionPoint = pointOnRay(rayThruScreen, t);

                    Vec3 prevProjection = project(prevIntersectionPoint - planePoint1, dragAxis);
                    Vec3 projection = project(intersectionPoint - planePoint1, dragAxis);

                    Vec3 delta = projection - prevProjection;

                    tc->position += delta;
                }
            }
        }
    }
    
    //
    // Check if entity was selected
    //
    if (mousePressed && !clickHandled)
    {
        RaycastResult rayResult = castRay(&game->activeScene->ecs, rayThruScreen);
        if (rayResult.hit)
        {
            editor->selectedEntity = rayResult.hitEntity;
        }
    }

    //
    // Show the component list
    //
    if (editor->selectedEntity.id != 0)
    {
        Entity e = editor->selectedEntity;
        
        // If the user clicks the X, it will deselect the entity. Look into changing this behavior
        bool componentUiClosed = false;
        
        // TODO: handle component groups
        UiReflector reflector;
        reflector.useLocalXfm = editor->isLocalXfm;

        if (!reflector.startReflection(e.friendlyName))
        {
            componentUiClosed = true;
        }

        // Get components
        TransformComponent* transform = getTransformComponent(e);
        ColliderComponent* collider = getColliderComponent(e);
        CameraComponent* camera = getCameraComponent(e);
        PortalComponent* portal = getPortalComponent(e);
        DirectionalLightComponent* directionalLights = getDirectionalLightComponent(e);
        auto pointLights = getPointLightComponents(e);
        auto renderComponents = getRenderComponents(e);

        //
        // Docked buttons
        //
        {
            bool sameLine = false;
            if (renderComponents.numComponents > 0 && transform != nullptr)
            {
                sameLine = true;
                ImGui::Checkbox("aabb", &game->editor.drawAabb);
            }

            if (collider != nullptr)
            {
                if (sameLine) ImGui::SameLine();
            
                ImGui::Checkbox("coll", &game->editor.drawCollider);
            }
        }

        //
        // Component reflection
        //
        {
            if (transform)
            {
                if (ImGui::CollapsingHeader("Transform"))
                {
                    reflector.setPrimaryReflectionTarget(transform);
                    reflectTransformComponent(&reflector, 0);
                }
            }
    
            if (collider)
            {
                if (ImGui::CollapsingHeader("Collider"))
                {
                    reflector.setPrimaryReflectionTarget(collider);
                    reflectColliderComponent(&reflector, collider, 0);
                }
            }

            // TODO
    
            // if (camera)
            // {
            //     if (ImGui::CollapsingHeader("Camera"))
            //     {
            //         reflector.reflectionTarget = camera;
            //         reflectCameraComponent(&reflector, camera, 0);
            //     }
            // }
    
            // if (directionalLight)
            // {
            //     if (ImGui::CollapsingHeader("Directional Light"))
            //     {
            //         reflector.reflectionTarget = directionalLight;
            //         reflectDirectionalLightComponent(&reflector, 0);
            //     }
            // }
    
            // if (pointLight)
            // {
            //     if (ImGui::CollapsingHeader("Point Light"))
            //     {
            //         reflector.reflectionTarget = pointLight;
            //         reflectPointLightComponent(&reflector, 0);
            //     }
            // }
    
            if (portal)
            {
                if (ImGui::CollapsingHeader("Portal"))
                {
                    reflector.setPrimaryReflectionTarget(portal);
                    reflectPortalComponent(&reflector, 0);
                }
            }
    
            // TODO
            // RenderComponent* t = getRenderComponent(e);
        }
    

        reflector.endReflection();

        if (componentUiClosed) editor->selectedEntity.id = 0;
    }
}
