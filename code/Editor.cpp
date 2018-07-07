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

    this->translator.xAxisHandle = &colliders.components[0];
    this->translator.yAxisHandle = &colliders.components[1];
    this->translator.zAxisHandle = &colliders.components[2];

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
    
    Ray rayThruScreen = rayThroughScreenCoordinate(getCameraComponent(game->activeCamera), Vec2(mouseX, mouseY));

    //
    // Scale and draw 3d translator handles
    //
    {
        TransformComponent* tc = getTransformComponent(editor->selectedEntity);

        if (tc)
        {
            TransformComponent* toolXfm = getTransformComponent(editor->translator.pseudoEntity);
            toolXfm->position = tc->position;
            
            // TODO: scale based on distance from camera
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
            DebugDraw::instance().drawArrow(tc->position, tc->position + arrowLength * Vec3(1, 0, 0));
            if (editor->translator.selectedHandle == 0)
            {
                DebugDraw::instance().drawCollider(editor->translator.xAxisHandle);
            }

            DebugDraw::instance().color = Vec3(0, 1, 0);
            DebugDraw::instance().drawArrow(tc->position, tc->position + arrowLength * Vec3(0, 1, 0));
            if (editor->translator.selectedHandle == 1)
            {
                DebugDraw::instance().drawCollider(editor->translator.yAxisHandle);
            }

            DebugDraw::instance().color = Vec3(0, 0, 1);
            DebugDraw::instance().drawArrow(tc->position, tc->position + arrowLength * Vec3(0, 0, 1));
            if (editor->translator.selectedHandle == 2)
            {
                DebugDraw::instance().drawCollider(editor->translator.zAxisHandle);
            }

            DebugDraw::instance().color = oldColor;

            //
            // Handle translator select 
            //
            if (mousePressed && !clickHandled)
            {
                for (uint32 i = 0; i < 3; i++)
                {
                    // raycast against x, y, and z colliders
                    
                    Aabb xAabb = Aabb(toolXfm->position + Vec3(Axis3D::X) * arrowLength / 2, scale * editor->translator.xAxisHandle->rect3Lengths / 2);
                    Aabb yAabb = Aabb(toolXfm->position + Vec3(Axis3D::Y) * arrowLength / 2, scale * editor->translator.yAxisHandle->rect3Lengths / 2);
                    Aabb zAabb = Aabb(toolXfm->position + Vec3(Axis3D::Z) * arrowLength / 2, scale * editor->translator.zAxisHandle->rect3Lengths / 2);

                    float32 minimumHit = FLT_MAX;

                    float32 t = rayAabbTest(rayThruScreen, xAabb);
                    if (t >= 0)
                    {
                        if (t < minimumHit)
                        {
                            minimumHit = t;
                            editor->translator.selectedHandle = 0;
                        }
                    }
                    
                    t = rayAabbTest(rayThruScreen, yAabb);
                    if (t >= 0)
                    {
                        if (t < minimumHit)
                        {
                            minimumHit = t;
                            editor->translator.selectedHandle = 1;
                        }
                    }
                    
                    t = rayAabbTest(rayThruScreen, zAabb);
                    if (t >= 0)
                    {
                        if (t < minimumHit)
                        {
                            minimumHit = t;
                            editor->translator.selectedHandle = 2;
                        }
                    }
                }
            }
            
            //
            // Handle translator drag
            //
            if (editor->translator.selectedHandle >= 0 && mouseHeld)
            {
                // project last mouse x, y against a plane defined by the dragging vector (and some other point?)
                // then project current mouse x, y on that plane
                // amount to translate is related to the length of the distance between those two vectors
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
        ComponentGroup<PointLightComponent> pointLights = getPointLightComponents(e);
        ComponentGroup<RenderComponent> renderComponents = getRenderComponents(e);

        //
        // Docked buttons
        //
        {
            bool sameLine = false;
            if (renderComponents.numComponents > 0 && transform != nullptr)
            {
                sameLine = true;
                ImGui::Checkbox("aabb", &game->editor.drawAabb);
            
                if (game->editor.drawAabb)
                {
                    DebugDraw::instance().drawAabb(e);
                }
            }

            if (collider != nullptr)
            {
                if (sameLine) ImGui::SameLine();
            
                ImGui::Checkbox("coll", &game->editor.drawCollider);
                if (game->editor.drawCollider)
                {
                    DebugDraw::instance().drawCollider(collider);
                }
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
                    reflector.reflectionTarget = transform;
                    reflectTransformComponent(&reflector, 0);
                }
            }
    
            if (collider)
            {
                if (ImGui::CollapsingHeader("Collider"))
                {
                    reflector.reflectionTarget = collider;
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
                    reflector.reflectionTarget = portal;
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
