#include "Editor.h"
#include "Game.h"
#include "Reflection.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "GLFW/glfw3.h"

#include "DebugDraw.h"

const char* EditorState::EntityListUi::DRAG_DROP_ID = "entity_drag_drop";

EditorState::EditorState()
{
    this->translator.pseudoEntity = makeEntity(&this->pseudoEcs, "Translator");
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
            if (editor->drawAabb)
            {
                DebugDraw::instance().drawAabb(e);
            }
                
            if (editor->drawCollider)
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
            toolXfm->setPosition(tc->position());

            if (editor->isLocalXfm)
            {
                toolXfm->setOrientation(tc->orientation());
            }
            
            TransformComponent* cameraXfm = getTransformComponent(game->activeCamera);

            const float32 SCALE_FACTOR = 0.1;

            float32 lengthFromCamera = length(tc->position() - cameraXfm->position());
            float32 scale = lengthFromCamera * SCALE_FACTOR;

            toolXfm->setScale(Vec3(scale));

            float32 arrowLength = editor->translator.xAxisHandle->rect3Lengths.x;
            arrowLength *= scale;
            
            Vec3 oldColor = DebugDraw::instance().color;

            glClear(GL_DEPTH_BUFFER_BIT);
            
            DebugDraw::instance().color = Vec3(1, 0, 0);
            DebugDraw::instance().drawArrow(tc->position(), tc->position() + arrowLength * toolXfm->right());

            DebugDraw::instance().color = Vec3(0, 1, 0);
            DebugDraw::instance().drawArrow(tc->position(), tc->position() + arrowLength * toolXfm->up());

            DebugDraw::instance().color = Vec3(0, 0, 1);
            DebugDraw::instance().drawArrow(tc->position(), tc->position() + arrowLength * toolXfm->back());

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
                
                Vec3 planePoint1 = tc->position();
                Vec3 planePoint2 = tc->position() + toolXfm->orientation() * Vec3(editor->translator.selectedHandle);

                Vec3 dragAxis = (planePoint2 - planePoint1).normalizeInPlace();
                Vec3 toCamera = cameraXfm->position() - tc->position();

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

                    tc->setPosition(tc->position() + delta);
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
        editor->componentList.thisFrameActiveId = 0;
            
        Entity e = editor->selectedEntity;
        
        // TODO: handle component groups
        UiReflector reflector;
        reflector.useLocalXfm = editor->isLocalXfm;
        reflector.editor = editor;

        reflector.startReflection(*getFriendlyName(e));
    
        ImGui::SetNextWindowSize(ImVec2(300, 500));
        ImGui::SetNextWindowPos(ImVec2(0, game->window->height - 500));
        ImGui::Begin((getFriendlyNameAndId(e) + "###e").cstr(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Get components
        EntityDetails* details = getEntityDetails(e);
        TransformComponent* transform = getTransformComponent(e);
        auto colliders = getColliderComponents(e);
        CameraComponent* camera = getCameraComponent(e);
        PortalComponent* portal = getPortalComponent(e);
        DirectionalLightComponent* directionalLight = getDirectionalLightComponent(e);
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
                ImGui::Checkbox("aabb", &editor->drawAabb);
            }

            if (colliders.numComponents > 0)
            {
               if (sameLine) ImGui::SameLine();
            
               ImGui::Checkbox("coll", &editor->drawCollider);
            }
        }

        //
        // Component reflection
        //
        {
            assert(details);
            {
                if (ImGui::Als_CollapsingHeaderTreeNode("Details"))
                {
                    reflector.setPrimaryReflectionTarget(details);
                    
                    // Sneak the id in here even though it is part of the entity, not the entity details
                    {
                        uint32 entityId = details->entity.id;
                        reflector.pushReflectionTarget(&entityId);
                        reflector.consumeUInt32("ID", 0, ReflectionFlag_ReadOnly);
                        reflector.popReflectionTarget();
                    }
                    
                    reflectEntityDetailsComponent(&reflector, 0);

                    ImGui::TreePop();
                }
            }
            
            if (transform)
            {
                if (ImGui::Als_CollapsingHeaderTreeNode("Transform"))
                {
                    reflector.setPrimaryReflectionTarget(transform);
                    reflectTransformComponent(&reflector, 0);

                    ImGui::TreePop();
                }
            }
    
            if (colliders.numComponents > 0)
            {
                bool multipleColliders = colliders.numComponents > 1;
                bool multipleColliderHeaderOpen = false;
                
                if (multipleColliders)
                {
                    char buffer[32];
                    sprintf_s(buffer, 32, "Colliders (%d)", colliders.numComponents);
                    multipleColliderHeaderOpen = ImGui::Als_CollapsingHeaderTreeNode(buffer);
                }

                if (!multipleColliders || multipleColliderHeaderOpen)
                {
                    for (uint32 i = 0; i < colliders.numComponents; i++)
                    {
                        ColliderComponent* cc = &colliders[i];
                        
                        bool xNotClicked = true;
                        char labelBuffer[32];
                        sprintf_s(labelBuffer, 32, "Collider##%d", i);
                        if (ImGui::Als_CollapsingHeaderTreeNode(labelBuffer, &xNotClicked))
                        {
                            reflector.setPrimaryReflectionTarget(cc);
                            reflectColliderComponent(&reflector, 0);

                            ImGui::TreePop();
                        }

                        if (!xNotClicked) removeColliderComponent(&cc);
                    }
                }

                if (multipleColliderHeaderOpen)
                {
                    ImGui::TreePop();
                }
            }

            // TODO
    
            if (camera)
            {
                bool headerOpen = false;
                bool xNotClicked = true;
                if (camera->entity.id == game->activeCamera.id)
                {
                    headerOpen = ImGui::Als_CollapsingHeaderTreeNode("Camera");
                }
                else
                {
                    headerOpen = ImGui::Als_CollapsingHeaderTreeNode("Camera", &xNotClicked);
                }
                
                if (headerOpen)
                {
                    reflector.setPrimaryReflectionTarget(camera);
                    reflectCameraComponent(&reflector, 0);

                    ImGui::TreePop();
                }

                if (!xNotClicked)
                {
                    // @TODO light this up
                    // removeCameraComponent(camera);
                }
            }
    
            if (directionalLight)
            {
                bool xNotClicked = true;
                if (ImGui::Als_CollapsingHeaderTreeNode("Directional Light", &xNotClicked))
                {
                    reflector.setPrimaryReflectionTarget(directionalLight);
                    reflectDirectionalLightComponent(&reflector, 0);

                    ImGui::TreePop();
                }

                if (!xNotClicked)
                {
                    removeDirectionalLightComponent(&directionalLight);
                    assert(directionalLight == nullptr);
                }
            }

            // @TODO
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
                bool xNotClicked = true;
                if (ImGui::Als_CollapsingHeaderTreeNode("Portal", &xNotClicked))
                {
                    uint32 connectedPortalIdBeforeReflect = portal->connectedPortalEntityId;
                    reflector.setPrimaryReflectionTarget(portal);
                    reflectPortalComponent(&reflector, 0);

                    if (portal->connectedPortalEntityId != connectedPortalIdBeforeReflect)
                    {
                        PortalComponent* newConnectedPortal = getPortalComponent(getEntity(game, portal->connectedPortalEntityId));
                        if (!newConnectedPortal)
                        {
                            // Entered ID doesn't have portal component, revert the update
                            portal->connectedPortalEntityId = connectedPortalIdBeforeReflect;
                        }
                        else
                        {
                            uint32 newConnectedPortalOldConnectionId = newConnectedPortal->connectedPortalEntityId;
                            newConnectedPortal->connectedPortalEntityId = e.id;

                            if (newConnectedPortalOldConnectionId != 0)
                            {
                                PortalComponent* unlinkedPortal = getPortalComponent(getEntity(game, newConnectedPortalOldConnectionId));
                                assert(unlinkedPortal != nullptr);

                                unlinkedPortal->connectedPortalEntityId = 0;
                            }
                        }
                    }

                    ImGui::TreePop();
                }

                if (!xNotClicked)
                {
                    removePortalComponent(&portal);
                    assert(portal == nullptr);
                }
            }
    
            // @TODO: other components
        }
    

        reflector.endReflection();
        ImGui::End();

        editor->componentList.lastFrameActiveId = editor->componentList.thisFrameActiveId;
    }

    //
    // Show the entity list
    //
    {
        struct // drawEntityAndChildren(..)
        {
            void operator () (Entity e, EditorState* editor)
            {
                // Set color to highlighted if entity is selected
                ImVec4 headerColor = e.id == (editor->selectedEntity.id) ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : ImGui::GetStyleColorVec4(ImGuiCol_Header);
                ImGui::PushStyleColor(ImGuiCol_Header, headerColor);

                typedef ImGuiTreeNodeFlags Flags;
                Flags flags = 0;
                flags |= ImGuiTreeNodeFlags_OpenOnArrow;
                flags |= ImGuiTreeNodeFlags_OpenOnArrow;
                if (getChildren(e)->size() == 0)       flags |= ImGuiTreeNodeFlags_Leaf;

                bool xNotClicked = true;
                bool open = ImGui::Als_CollapsingHeaderTreeNode(getFriendlyNameAndId(e).cstr(), &xNotClicked, flags);
                bool toggled = ImGui::Als_IsTreeNodeToggled();

                if (ImGui::IsItemDeactivated() && ImGui::IsItemHovered() && !toggled)
                {
                    // @Hack: This is the best way I could find to detect if the header was clicked but NOT opened
                    editor->selectedEntity = e;
                }

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload(EditorState::EntityListUi::DRAG_DROP_ID, &e, sizeof(Entity));
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorState::EntityListUi::DRAG_DROP_ID))
                    {
                        Entity dragged = *(Entity*)payload->Data;

                        if (dragged.id != e.id)
                        {
                            setParent(dragged, e);
                        }
                    }
                }

                if (open)
                {
                    for (Entity child : *getChildren(e))
                    {
                        (*this)(child, editor); // recursively draw children
                    }

                    ImGui::TreePop();
                }

                if (!xNotClicked)
                {
                    // TODO: Add this to a list of entities to be removed at the end of the frame... don't want to remove it while iterating!
                    //       Also, should we remove the children entities too? I suspect so.
                }

                ImGui::PopStyleColor();
            }
        } drawEntityAndChildren;

        ImGui::SetNextWindowSize(ImVec2(300, 500));
        ImGui::SetNextWindowPos(ImVec2(game->window->width - 300, game->window->height - 500));
        ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::Text("(drag here to unparent)");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorState::EntityListUi::DRAG_DROP_ID))
            {
                Entity dragged = *(Entity*)payload->Data;
                removeParent(dragged);
            }
        }
        
        // ImGui::PushItemWidth(-1);
        {   
            for (Entity e : game->activeScene->ecs.entities)
            {
                // Skip entity if it has a parent
                TransformComponent* xfm = getTransformComponent(e);
                if (xfm && getParent(e).id != 0) continue;

                drawEntityAndChildren(e, editor);
            }
        }
        // ImGui::PopItemWidth();
                
        ImGui::End();
    }
}
