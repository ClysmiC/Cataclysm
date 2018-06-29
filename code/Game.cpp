#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <iostream>
#include <thread>

#include "Ecs.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "DebugDraw.h"
#include "Mesh.h"
#include "GL/glew.h"
#include "Ray.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "DebugGlobal.h"
#include "Reflection.h"

#include "als_fixed_string.h"

bool keys[1024];
bool lastKeys[1024];

bool mouseButtons[8];
bool lastMouseButtons[8];

float mouseX;
float mouseY;
float mouseXPrev;
float mouseYPrev;

float32 timeMs;
float32 deltaTMs;

Ray lastCastRay;
bool hasCastRayYet = false;

///////////////////////////////////////////////////////////////////////
//////////// BEGIN SCRATCHPAD (throwaway or refactorable code)

void updateLastKeysAndMouseButtons()
{
    for (uint32 i = 0; i < 1024; i++)
    {
        lastKeys[i] = keys[i];
    }

    for (uint32 i = 0; i < 8; i++)
    {
        lastMouseButtons[i] = mouseButtons[i];
    }
}

void buildTestScene1(Scene* scene)
{
    ResourceManager& rm = ResourceManager::instance();
    
    Mesh *hexMesh = rm.initMesh("hex/hex.obj", true, true);
    Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, true);

    // Set up directional light
    {
        Entity dirLight = makeEntity(&scene->ecs, "directional light");
        DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

        dlc->intensity = Vec3(.25, .25, .25);
        dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
    }

    //
    // Set up cubemap
    //
    Cubemap* cm = rm.initCubemap("cubemap/watersky", ".jpg", true);
    addCubemap(scene, cm);

    //
    // Set up hex meshes
    //
    const uint32 hexCount = 2;
    Vec3 hexPositions[hexCount];
    hexPositions[0] = Vec3(-18, 0, -40);
    hexPositions[1] = Vec3(6, 0, -4);

    for (uint32 i = 0; i < hexCount; i++)
    {
        Entity e = makeEntity(&scene->ecs, "hex");
        
        TransformComponent *tc = addTransformComponent(e);
        tc->position = hexPositions[i];
        tc->scale = Vec3(.25);
        
        ComponentGroup<RenderComponent> rcc = addRenderComponents(e, hexMesh->submeshes.size());

        for(uint32 j = 0; j < hexMesh->submeshes.size(); j++)
        {
            RenderComponent *rc = rcc.components + j;
            new (rc) RenderComponent(e, &(hexMesh->submeshes[j]));
        }
    }
}

void buildTestScene2(Scene* scene)
{
    ResourceManager& rm = ResourceManager::instance();
    
    Mesh *icosahedronMesh = rm.initMesh("icosahedron/icosahedron.obj", true, true);

    // Set up directional light
    {
        Entity dirLight = makeEntity(&scene->ecs, "directional light");
        DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

        dlc->intensity = Vec3(.25, .25, .25);
        dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
    }

    //
    // Set up cubemap
    //
    Cubemap* cm = rm.initCubemap("cubemap/CloudyLightRays", ".png", true);
    addCubemap(scene, cm);

    //
    // Set up icosahedron meshes
    //
    const uint32 icosahedronCount = 4;
    Vec3 icosahedronPositions[icosahedronCount];
    icosahedronPositions[0] = Vec3(-4, 2, 3);
    icosahedronPositions[1] = Vec3(-5, 4, 3);
    icosahedronPositions[2] = Vec3(-6, 0, 3);
    icosahedronPositions[3] = Vec3(-2, 2, 1);

    for (uint32 i = 0; i < icosahedronCount; i++)
    {
        Entity e = makeEntity(&scene->ecs, "icosahedron");
        TransformComponent *tc = addTransformComponent(e);
        tc->position = icosahedronPositions[i];

        ComponentGroup<RenderComponent> rcc = addRenderComponents(e, icosahedronMesh->submeshes.size());

        ColliderComponent *cc = addColliderComponent(e);
        new (cc) ColliderComponent(icosahedronMesh->bounds);
                                   
        for(uint32 j = 0; j < icosahedronMesh->submeshes.size(); j++)
        {
            RenderComponent *rc = rcc.components + j;
            new (rc) RenderComponent(e, &(icosahedronMesh->submeshes[j]));
        }
    }
}

void buildTestScene3(Scene* scene)
{
    ResourceManager& rm = ResourceManager::instance();
    
    Mesh *shuttleMesh = rm.initMesh("shuttle/shuttle.obj", true, true);
    Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, true);

    // Set up directional light
    {
        Entity dirLight = makeEntity(&scene->ecs, "directional light");
        DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

        dlc->intensity = Vec3(.25, .25, .25);
        dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
    }

    //
    // Set up cubemap
    //
    Cubemap* cm = rm.initCubemap("cubemap/SunSet", ".png", true);
    addCubemap(scene, cm);

    //
    // Set up shuttle meshes
    //
    const uint32 shuttleCount = 1;
    Vec3 shuttlePositions[shuttleCount];
    shuttlePositions[0] = Vec3(0, 0, -10);

    for (uint32 i = 0; i < shuttleCount; i++)
    {
        Entity e = makeEntity(&scene->ecs, "shuttle");
        TransformComponent *tc = addTransformComponent(e);
        tc->position = shuttlePositions[i];
        tc->scale = Vec3(.25);
        
        ComponentGroup<RenderComponent> rcc = addRenderComponents(e, shuttleMesh->submeshes.size());

        for(uint32 j = 0; j < shuttleMesh->submeshes.size(); j++)
        {
            RenderComponent *rc = rcc.components + j;
            new (rc) RenderComponent(e, &(shuttleMesh->submeshes[j]));
        }
    }
}

void updateCameraXfm(Game* game)
{
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

    bool draggingCameraInEditMode = game->isEditorMode && mouseButtons[GLFW_MOUSE_BUTTON_1] && !ImGui::GetIO().WantCaptureMouse;

    if (!game->isEditorMode || draggingCameraInEditMode)
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
}

void updateGame(Game* game)
{
    updateCameraXfm(game);
    
    //
    // Rotate/scale test entity
    TransformComponent* testXfm = getTransformComponent(game->editor.selectedEntity);

    if (testXfm)
    {
        // float32 timeS = timeMs / 1000.0f;

        // testXfm->scale.x = .5 + .25 * sinf(timeS / 2.0f);
        // testXfm->scale.y = .5 + .25 * cosf(timeS / 6.0f);
        // testXfm->scale.z = .5 + .25 * cosf(timeS / 10.0f);

        // testXfm->orientation = axisAngle(Vec3(3, 1, 1), timeS * 6);
    }

    assert(game->activeScene != nullptr);
    CameraComponent* camComponent = getCameraComponent(game->activeCamera);
    TransformComponent* camXfm = getTransformComponent(game->activeCamera);

    if (keys[GLFW_KEY_GRAVE_ACCENT] && !lastKeys[GLFW_KEY_GRAVE_ACCENT])
    {
        game->isEditorMode = !game->isEditorMode;

        if (game->isEditorMode)
        {
            glfwSetInputMode(game->window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(game->window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    
    if (game->isEditorMode && mouseButtons[GLFW_MOUSE_BUTTON_1] && !lastMouseButtons[GLFW_MOUSE_BUTTON_1] && !ImGui::GetIO().WantCaptureMouse)
    {
        lastCastRay = rayThroughScreenCoordinate(getCameraComponent(game->activeCamera), Vec2(mouseX, mouseY));
        hasCastRayYet = true;

        RaycastResult rayResult = castRay(&game->activeScene->ecs, lastCastRay);
        if (rayResult.hit)
        {
            game->editor.selectedEntity = rayResult.hitEntity;
        }
    }

    renderScene(game->activeScene, camComponent, camXfm);

    if (game->isEditorMode)
    {
        for (Entity& e : game->activeScene->ecs.entities)
        {
            if (e.id == game->editor.selectedEntity.id)
            {
                if (testUiReflection(game, e))
                {
                    // User didn't X out of window
                }
                else
                {
                    // User X'd out of window
                    game->editor.selectedEntity.id = 0;
                }
            }
        }
    }
    
    if (hasCastRayYet)
    {
        // DebugDraw::instance().drawLine(lastCastRay.position, lastCastRay.position + 10 * lastCastRay.direction, camComponent, camXfm);
    }
}

// END SCRATCHPAD
///////////////////////////////////////////////////////////////////

Scene* makeScene(Game* game)
{
    assert(game->numScenes < MAX_SCENES);
    Scene* result = game->scenes + game->numScenes;
    
    new (result) Scene();

    result->game = game;
    game->numScenes++;

    return result;
}

void makeSceneActive(Game* game, Scene* scene)
{
    game->activeScene = scene;
}

void makeCameraActive(Game* game, Entity camera)
{
    assert(getCameraComponent(camera) != nullptr);
    assert(getTransformComponent(camera) != nullptr);

    game->activeCamera = camera;
}

int main()
{    
    // INIT WINDOW
    uint32 windowWidth = 1024;
    uint32 windowHeight = 720;

    // uint32 windowWidth = 1920;
    // uint32 windowHeight = 1080;
    
    Window window;

    if(!initGlfwWindow(&window, windowWidth, windowHeight))
    {
        return -1;
    }

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO* io = &ImGui::GetIO();
    io->IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow, true);
    ImGui_ImplOpenGL3_Init();
    
    ImGui::StyleColorsDark();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    ResourceManager::instance().init();
    DebugDraw::instance().init();
    
    float32 lastTimeMs = 0;

    Game* game = new Game();
    game->isEditorMode = false;
    game->window = &window;
    
    Scene* testScene1 = makeScene(game);
    buildTestScene1(testScene1);
    
    Scene* testScene2 = makeScene(game);
    buildTestScene2(testScene2);

    Scene* testScene3 = makeScene(game);
    buildTestScene3(testScene3);
    
    // Set up camera
    Entity camera = makeEntity(&testScene1->ecs, "camera");
    TransformComponent* cameraXfm = addTransformComponent(camera);
    CameraComponent* cameraComponent = addCameraComponent(camera);
    cameraComponent->window = &window;
    
#if 1
    cameraComponent->isOrthographic = false;
    cameraComponent->perspectiveFov = 60.0f;
#else
    cameraComponent->isOrthographic = true;
    cameraComponent->orthoWidth = 10.0f;
#endif
    
    cameraComponent->aspectRatio = 4.0f / 3.0f;
    cameraComponent->near = 0.01f;
    cameraComponent->far = 1000.0f;
    recalculateProjectionMatrix(cameraComponent);

    DebugDraw::instance().cameraComponent = cameraComponent;
    DebugDraw::instance().cameraXfm = cameraXfm;

    ColliderComponent* debugCC;
    
    //
    // Set up portal from scene 1<->2
    //
    {
        Entity portal1 = makeEntity(&testScene1->ecs, "portalA1");
        Entity portal2 = makeEntity(&testScene2->ecs, "portalA2");

        Transform portal1Xfm;
        portal1Xfm.position = Vec3(0, 0, -10);
        portal1Xfm.orientation = axisAngle(Vec3(0, 1, 0), 180);
        
        Transform portal2Xfm;
        portal2Xfm.position = Vec3(1, 2, 3);
        portal2Xfm.orientation = axisAngle(Vec3(0, 1, 0), 45);

        Vec2 dimensions(2, 3);
        
        createPortalFromTwoBlankEntities(portal1, portal2, portal1Xfm, portal2Xfm, dimensions);

        debugCC = getColliderComponent(portal1);
    }

    //
    // Set up portal from scene 1<->3
    //
    {
        Entity portal1 = makeEntity(&testScene1->ecs, "portalB1");
        Entity portal3 = makeEntity(&testScene3->ecs, "portalB3");

        Transform portal1Xfm;
        portal1Xfm.position = Vec3(0, 0, -10);
        portal1Xfm.orientation = axisAngle(Vec3(0, 1, 0), 0);
        
        Transform portal3Xfm;
        portal3Xfm.position = Vec3(0, 0, 0);
        portal3Xfm.orientation = axisAngle(Vec3(0, 1, 0), 0);

        Vec2 dimensions(2, 3);
        
        createPortalFromTwoBlankEntities(portal1, portal3, portal1Xfm, portal3Xfm, dimensions);
    }

    makeSceneActive(game, testScene1);
    makeCameraActive(game, camera);
    
    while(!glfwWindowShouldClose(window.glfwWindow))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

#if 0
        ImGui::ShowDemoWindow();
#endif
        
        glClearColor(.5f, 0.5f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        float32 timeS = glfwGetTime();
        timeMs = timeS * 1000.0f;
        deltaTMs = timeMs - lastTimeMs;
        lastTimeMs = timeMs;

        if (keys[GLFW_KEY_1] && !lastKeys[GLFW_KEY_1])
        {
            debug_hidePortalContents = !debug_hidePortalContents;
        }
        
        updateGame(game);

        // DebugDraw::instance().drawCollider(debugCC, cameraComponent, cameraXfm);

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window.glfwWindow);

        //
        //
        //
        // After Swap Buffers
        //
        //
        //

        mouseXPrev = mouseX;
        mouseYPrev = mouseY;

        updateLastKeysAndMouseButtons();

        
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window.glfwWindow);
    glfwTerminate();
    return 0;
}
