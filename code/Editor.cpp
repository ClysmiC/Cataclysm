#include "Editor.h"
#include "Game.h"
#include "Reflection.h"
#include "Window.h"

#include "als/als_util.h"
#include "als/als_temp_alloc.h"

#include "imgui/imgui.h"
#include "GLFW/glfw3.h"

#include "Quickhull.h"
#include "DebugDraw.h"
#include "platform/platform.h"

#include "resource/ResourceManager.h"
#include "resource/resources/Mesh.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/ConvexHullColliderComponent.h"
#include "ecs/components/ColliderComponent.h"
#include "ecs/components/CameraComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/DirectionalLightComponent.h"
#include "ecs/components/PointLightComponent.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/AgentComponent.h"
#include "ecs/components/WalkComponent.h"
#include "ecs/components/TerrainComponent.h"

#include <thread>

const char* EditorState::EntityListUi::DRAG_DROP_ID              = "entity_drag_drop";
const char* EditorState::EntityListUi::ADD_ENTITY_POPUP_ID       = "add_entity";
const char* EditorState::ComponentListUi::ADD_COMPONENT_POPUP_ID = "add_component";
const char* EditorState::ComponentListUi::ADD_RENDER_COMPONENT_POPUP_ID = "Select Mesh##add_render_component";

EditorState::EditorState()
{
    this->translator.pseudoEntity = makeEntity(&this->pseudoEcs, "Translator");
    auto colliders = addComponents<ColliderComponent>(this->translator.pseudoEntity, 3);

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

void selectEntity(EditorState* editor, Entity e)
{
    editor->selectedEntity = e;
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

    Ray prevRayThruScreen = rayThroughScreenCoordinate(getComponent<CameraComponent>(game->activeCamera), Vec2(mouseXPrev, mouseYPrev));
    Ray rayThruScreen = rayThroughScreenCoordinate(getComponent<CameraComponent>(game->activeCamera), Vec2(mouseX, mouseY));

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
        }
    }

    //
    // Draw convex hull (debug)
    //
    {
        Entity e = editor->selectedEntity;

        if (e.id != 0)
        {
            ConvexHullColliderComponent* chcc = getComponent<ConvexHullColliderComponent>(e);

            if (chcc && chcc->showInEditor)
            {
                TransformComponent* xfm = getComponent<TransformComponent>(e);

                for (uint32 i = 0; i < chcc->edges.size(); i++)
                {
                    DebugDraw::instance().drawLine(
                        (xfm->matrix() * Vec4(chcc->positions[chcc->edges[i].index0], 1)).xyz(),
                        (xfm->matrix() * Vec4(chcc->positions[chcc->edges[i].index1], 1)).xyz()
                    );
                }
            }
        }
    }


    //
    // Scale and draw 3d translator handles
    //
    {
        TransformComponent* tc = getComponent<TransformComponent>(editor->selectedEntity);

        if (tc)
        {
            TransformComponent* toolXfm = getComponent<TransformComponent>(editor->translator.pseudoEntity);
            toolXfm->setPosition(tc->position());

            if (editor->isLocalXfm)
            {
                toolXfm->setOrientation(tc->orientation());
            }
            
            TransformComponent* cameraXfm = getComponent<TransformComponent>(game->activeCamera);

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
        RaycastResult rayResult = castRay(game->activeScene, rayThruScreen);
        if (rayResult.hit)
        {
            selectEntity(editor, rayResult.hitEntity);
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
        EntityDetails* details = getComponent<EntityDetails>(e);
        TransformComponent* transform = getComponent<TransformComponent>(e);
        auto colliders = getComponents<ColliderComponent>(e);
        CameraComponent* camera = getComponent<CameraComponent>(e);
        PortalComponent* portal = getComponent<PortalComponent>(e);
        auto directionalLights = getComponents<DirectionalLightComponent>(e);
        auto pointLights = getComponents<PointLightComponent>(e);
        auto renderComponents = getComponents<RenderComponent>(e);
        auto convexHullColliders = getComponents<ConvexHullColliderComponent>(e);

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
        // Add component button
        //
        if (ImGui::Button("Add Component (+)"))
        {
            ImGui::OpenPopup(EditorState::ComponentListUi::ADD_COMPONENT_POPUP_ID);
        }

        if (ImGui::BeginPopup(EditorState::ComponentListUi::ADD_COMPONENT_POPUP_ID))
        {

            // Note: Use short-circuit evaluation to not show selectable for components
            //       that we can't add (e.g., if we already have a camera component)
            if (ImGui::Selectable("Collider"))          addComponent<ColliderComponent>(e);
            if (!camera && ImGui::Selectable("Camera")) addComponent<CameraComponent>(e);
            if (!portal && ImGui::Selectable("Portal")) addComponent<PortalComponent>(e);
            if (ImGui::Selectable("Directional Light")) addComponent<DirectionalLightComponent>(e);
            if (ImGui::Selectable("Point Light"))       addComponent<PointLightComponent>(e);

            // Select mesh modal window
            {
                // Use short-circuit evaluation
                if (renderComponents.numComponents == 0 && ImGui::Selectable("Render Mesh", false, ImGuiSelectableFlags_DontClosePopups)) // @Hack: If the selectable closes, the below code only runs 1 frame. If we define the popup elsewhere, the ID stacks are incompatible. Easiest solution, just keep this popup open.
                {
                    ImGui::OpenPopup(EditorState::ComponentListUi::ADD_RENDER_COMPONENT_POPUP_ID);
                }

                if (ImGui::BeginPopupModal(EditorState::ComponentListUi::ADD_RENDER_COMPONENT_POPUP_ID))
                {
                    auto& meshPopup = editor->componentList.meshFileSelection;

                    bool closePopup = false;
                    if (editor->componentList.meshFileSelection.isLoadingMesh)
                    {
                        ImGui::Text("Loading %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                        
                        if (editor->componentList.meshFileSelection.finishedLoadingMesh)
                        {
                            editor->componentList.meshFileSelection.isLoadingMesh = false;
                            editor->componentList.meshFileSelection.finishedLoadingMesh = false;
                            
                            Mesh* m = ResourceManager::instance().getMesh(meshPopup.meshFilesPtrs[meshPopup.selectedIndex]);
                            assert(m->isLoaded);

                            if (!isUploadedToGpuOpenGl(m)) uploadToGpuOpenGl(m);

                            auto rcc = addComponents<RenderComponent>(e, m->submeshes.size());
                            initRenderComponents(&rcc, m);

                            closePopup = true;
                        }
                    }
                    else
                    {
                        if (!meshPopup.isOpen || meshPopup.refreshFileList)
                        {
                            if (!meshPopup.isOpen)
                            {
                                // Only re-search for file if the popup was just opened. If it was just a filter update,
                                // only filter the results, don't re-search the OS for files
                                meshPopup.meshFiles = getAllFileNames(ResourceManager::instance().resourceDirectory.cstr(), true, "obj");
                            }

                            meshPopup.meshFilesCount = 0;
                            meshPopup.isOpen = true;
                            meshPopup.refreshFileList = false;


                            for (uint32 i = 0; i < meshPopup.meshFiles.size(); i++)
                            {
                                if (i >= ARRAY_LEN(meshPopup.meshFilesPtrs))
                                {
                                    assert(false);
                                    break;
                                }

                                if (meshPopup.filter == "" || meshPopup.meshFiles[i].find(meshPopup.filter.cstr(), 0) != std::string::npos)
                                {
                                    meshPopup.meshFilesPtrs[meshPopup.meshFilesCount] = meshPopup.meshFiles[i].c_str();
                                    meshPopup.meshFilesCount++;
                                }
                            }
                        }

                        ImGui::PushItemWidth(500);
                        if (ImGui::InputText("Filter", meshPopup.filter.data, 256))
                        {
                            meshPopup.filter.invalidateLength();
                            meshPopup.refreshFileList = true;
                        }

                        ImGui::PushItemWidth(500);
                        ImGui::ListBox("Mesh File", &meshPopup.selectedIndex, meshPopup.meshFilesPtrs, meshPopup.meshFilesCount, 6);
                        ImGui::PopItemWidth();

                        if (ImGui::Button("Add (+)"))
                        {
                            editor->componentList.meshFileSelection.isLoadingMesh = true;

                            // Asynchronously load mesh
                            std::thread meshLoader([&meshPopup] {
                                    ResourceManager& rm = ResourceManager::instance();
                                    rm.initMesh(meshPopup.meshFilesPtrs[meshPopup.selectedIndex], true, MeshLoadOptions::CPU); // CPU only because we need to upload to GPU on the main thread
                                    meshPopup.finishedLoadingMesh = true;
                                }
                            );

                            meshLoader.detach();
                        }

                        ImGui::SameLine();

                        if (ImGui::Button("Close"))
                        {
                            closePopup = true;
                        }
                        
                    }

                    ImGui::EndPopup();
                    if (closePopup) ImGui::CloseCurrentPopup(); // Closes both mesh and add component popup (see @Hack about why we left this popup open)
                }
            }

            ImGui::EndPopup();
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
                    sprintf_s(buffer, 32, "Prim. Colliders (%d)", colliders.numComponents);
                    multipleColliderHeaderOpen = ImGui::Als_CollapsingHeaderTreeNode(buffer);
                }

                if (!multipleColliders || multipleColliderHeaderOpen)
                {
                    for (uint32 i = 0; i < colliders.numComponents; i++)
                    {
                        ColliderComponent* cc = &colliders[i];
                        
                        bool xNotClicked = true;
                        char labelBuffer[32];
                        sprintf_s(labelBuffer, 32, "Prim. Collider##%d", i);
                        if (ImGui::Als_CollapsingHeaderTreeNode(labelBuffer, &xNotClicked))
                        {
                            reflector.setPrimaryReflectionTarget(cc);
                            reflectColliderComponent(&reflector, 0);

                            ImGui::TreePop();
                        }

                        if (!xNotClicked) removeComponent<ColliderComponent>(&cc);
                    }
                }

                if (multipleColliderHeaderOpen)
                {
                    ImGui::TreePop();
                }
            }

            if (convexHullColliders.numComponents > 0)
            {
                bool multipleColliders = convexHullColliders.numComponents > 1;
                bool multipleColliderHeaderOpen = false;
                
                if (multipleColliders)
                {
                    char buffer[32];
                    sprintf_s(buffer, 32, "Conv. Colliders (%d)", convexHullColliders.numComponents);
                    multipleColliderHeaderOpen = ImGui::Als_CollapsingHeaderTreeNode(buffer);
                }

                if (!multipleColliders || multipleColliderHeaderOpen)
                {
                    for (uint32 i = 0; i < convexHullColliders.numComponents; i++)
                    {
                        ConvexHullColliderComponent* chcc = &convexHullColliders[i];
                        
                        bool xNotClicked = true;
                        char labelBuffer[32];
                        sprintf_s(labelBuffer, 32, "Conv. Collider##%d", i);
                        if (ImGui::Als_CollapsingHeaderTreeNode(labelBuffer, &xNotClicked))
                        {
                            reflector.setPrimaryReflectionTarget(chcc);
                            reflectConvexHullColliderComponent(&reflector, 0);

                            ImGui::TreePop();
                        }

                        if (!xNotClicked) removeComponent(&chcc);
                    }
                }

                if (multipleColliderHeaderOpen)
                {
                    ImGui::TreePop();
                }
            }
    
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
                    // removeComponent<CameraComponent>(camera);
                }
            }
    
            if (directionalLights.numComponents > 0)
            {
                bool multiple = directionalLights.numComponents > 1;
                bool multipleHeaderOpen = false;
                
                if (multiple)
                {
                    char buffer[32];
                    sprintf_s(buffer, 32, "Directional Lights (%d)", directionalLights.numComponents);
                    multipleHeaderOpen = ImGui::Als_CollapsingHeaderTreeNode(buffer);
                }

                if (!multiple || multipleHeaderOpen)
                {
                    for (uint32 i = 0; i < directionalLights.numComponents; i++)
                    {
                        DirectionalLightComponent* component = &directionalLights[i];
                        
                        bool xNotClicked = true;
                        char labelBuffer[32];
                        sprintf_s(labelBuffer, 32, "Directional Light##%d", i);
                        if (ImGui::Als_CollapsingHeaderTreeNode(labelBuffer, &xNotClicked))
                        {
                            reflector.setPrimaryReflectionTarget(component);
                            reflectDirectionalLightComponent(&reflector, 0);

                            ImGui::TreePop();
                        }

                        if (!xNotClicked) removeComponent<DirectionalLightComponent>(&component);
                    }
                }

                if (multipleHeaderOpen)
                {
                    ImGui::TreePop();
                }
            }

            if (pointLights.numComponents > 0)
            {
                bool multiple = pointLights.numComponents > 1;
                bool multipleHeaderOpen = false;
                
                if (multiple)
                {
                    char buffer[32];
                    sprintf_s(buffer, 32, "Point Lights (%d)", pointLights.numComponents);
                    multipleHeaderOpen = ImGui::Als_CollapsingHeaderTreeNode(buffer);
                }

                if (!multiple || multipleHeaderOpen)
                {
                    for (uint32 i = 0; i < pointLights.numComponents; i++)
                    {
                        PointLightComponent* component = &pointLights[i];
                        
                        bool xNotClicked = true;
                        char labelBuffer[32];
                        sprintf_s(labelBuffer, 32, "Point Light##%d", i);
                        if (ImGui::Als_CollapsingHeaderTreeNode(labelBuffer, &xNotClicked))
                        {
                            reflector.setPrimaryReflectionTarget(component);
                            reflectPointLightComponent(&reflector, 0);

                            ImGui::TreePop();
                        }

                        if (!xNotClicked) removeComponent<PointLightComponent>(&component);
                    }
                }

                if (multipleHeaderOpen)
                {
                    ImGui::TreePop();
                }
            }
    
            if (portal)
            {
                bool xNotClicked = true;
                if (ImGui::Als_CollapsingHeaderTreeNode("Portal", &xNotClicked))
                {
                    PotentiallyStaleEntity connectedPortalBeforeReflect = portal->connectedPortal;
                    reflector.setPrimaryReflectionTarget(portal);
                    reflectPortalComponent(&reflector, 0);

                    if (portal->connectedPortal.id != connectedPortalBeforeReflect.id)
                    {
                        PortalComponent* newConnectedPortal = getComponent<PortalComponent>(getEntity(getGame(portal->entity), &portal->connectedPortal));
                        if (!newConnectedPortal)
                        {
                            portal->connectedPortal = connectedPortalBeforeReflect;
                        }
                        else
                        {
                            PotentiallyStaleEntity newPortalOldConnected = newConnectedPortal->connectedPortal;
                            newConnectedPortal->connectedPortal = e;

                            if (newPortalOldConnected.id != 0)
                            {
                                PortalComponent* unlinkedPortal = getComponent<PortalComponent>(getEntity(getGame(portal->entity), &newPortalOldConnected));
                                assert(unlinkedPortal != nullptr);

                                unlinkedPortal->connectedPortal.id = 0;
                            }
                        }
                    }

                    ImGui::TreePop();
                }

                if (!xNotClicked)
                {
                    removeComponent<PortalComponent>(&portal);
                    assert(portal == nullptr);
                }
            }

            if (renderComponents.numComponents > 0)
            {
                // Just reflecting 1 render-component will give us all of the mesh information we need
                bool xNotClicked = true;
                bool isVisibleToggled = false;
                if (ImGui::Als_CollapsingHeaderTreeNode("Render Mesh", &xNotClicked))
                {
                    bool isVisibleBeforeReflect = renderComponents[0].isVisible;
                    reflector.setPrimaryReflectionTarget(&renderComponents[0]);
                    reflectRenderComponent(&reflector, 0);

                    isVisibleToggled = (isVisibleBeforeReflect != renderComponents[0].isVisible);

                    ImGui::TreePop();
                }

                if (isVisibleToggled)
                {
                    for (uint32 i = 1; i < renderComponents.numComponents; i++)
                    {
                        renderComponents[i].isVisible = renderComponents[0].isVisible;
                    }
                }

                if (!xNotClicked)
                {
                    for (uint32 i = 0; i < renderComponents.numComponents; i++)
                    {
                        RenderComponent* rc = &renderComponents[i];
                        removeComponent<RenderComponent>(&rc);
                        assert(rc == nullptr);
                    }
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
                ImVec4 headerColor = (e.id == editor->selectedEntity.id) ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : ImGui::GetStyleColorVec4(ImGuiCol_Header);
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
                    selectEntity(editor, e);
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
                    for (PotentiallyStaleEntity child : *getChildren(e))
                    {
                        (*this)(getEntity(getGame(e), &child), editor); // recursively draw children
                    }

                    ImGui::TreePop();
                }

                if (!xNotClicked)
                {
                    markEntityForDeletion(e);
                }

                ImGui::PopStyleColor();
            }
        } drawEntityAndChildren;

        ImGui::SetNextWindowSize(ImVec2(300, 500));
        ImGui::SetNextWindowPos(ImVec2(game->window->width - 300, game->window->height - 500));
        ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        if(ImGui::Button("Add Entity (+)"))
        {
            ImGui::OpenPopup(EditorState::EntityListUi::ADD_ENTITY_POPUP_ID);
        }

        if (ImGui::BeginPopup(EditorState::EntityListUi::ADD_ENTITY_POPUP_ID))
        {
            if (ImGui::Selectable("Blank Entity"))
            {
                Entity e = makeEntity(&game->activeScene->ecs, "New Entity");
                selectEntity(editor, e);
            }

            // TODO: Add new entity "templates" here
            
            ImGui::EndPopup();
        }
        
        ImGui::Text("(drag here to unparent)");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorState::EntityListUi::DRAG_DROP_ID))
            {
                Entity dragged = *(Entity*)payload->Data;
                removeParent(dragged);
            }
        }
        
        {   
            for (Entity e : game->activeScene->ecs.entities)
            {
                // Skip entity if it has a parent
                TransformComponent* xfm = getComponent<TransformComponent>(e);
                if (xfm && getParent(e).id != 0) continue;

                drawEntityAndChildren(e, editor);
            }
        }
                
        ImGui::End();
    }
}
