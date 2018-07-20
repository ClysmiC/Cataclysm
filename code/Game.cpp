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
    scene->name = "Test Scene 1 (hex)";

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
        ColliderComponent* cc = addColliderComponent(e);
        tc->setWorldPosition(hexPositions[i]);
        tc->setWorldScale(Vec3(.25));
        
        auto rcc = addRenderComponents(e, hexMesh->submeshes.size());

        for(uint32 j = 0; j < hexMesh->submeshes.size(); j++)
        {
            RenderComponent *rc = &rcc[j];
            new (rc) RenderComponent(e, &(hexMesh->submeshes[j]));
        }
    }

    //
    // Set up terrain
    //
    {
        Entity e = makeEntity(&scene->ecs, "terrain");

        // TODO: rendering code expects entities with rendercomponents to also have transforms.
        //       we could assume the origin at this case, or maybe set a flag on the entity that explicitly
        //       says we don't store a transform so we can have the robustness of that check without the memory
        //       overhead
        TransformComponent *xfm = addTransformComponent(e);
        xfm->setWorldPosition(Vec3(0, 0, 0));
        
        TerrainComponent* tc = addTerrainComponent(e);
        new (tc) TerrainComponent("heightmap.bmp", Vec3(-200, 0, -200), 400, 400, -10, 8);

        uint32 numChunks = tc->xChunkCount * tc->zChunkCount;

        auto rcList = addRenderComponents(e, numChunks);

        for (uint32 i = 0; i < tc->zChunkCount; i++)
        {
            for (uint32 j = 0; j < tc->xChunkCount; j++)
            {
                RenderComponent* rc = &rcList[i * tc->xChunkCount + j];

                TerrainChunk* chunk = &tc->chunks[i][j];
                new (rc) RenderComponent(e, &chunk->mesh.submeshes[0]);
            }
        }
    }
}

void buildTestScene2(Scene* scene)
{
    scene->name = "Test Scene 2 (ico)";

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
        tc->setWorldPosition(icosahedronPositions[i]);

        auto rcc = addRenderComponents(e, icosahedronMesh->submeshes.size());

        ColliderComponent *cc = addColliderComponent(e);
        new (cc) ColliderComponent(icosahedronMesh->bounds);
                                   
        for(uint32 j = 0; j < icosahedronMesh->submeshes.size(); j++)
        {
            RenderComponent *rc = &rcc[j];
            new (rc) RenderComponent(e, &(icosahedronMesh->submeshes[j]));
        }
    }
}

void buildTestScene3(Scene* scene)
{
    scene->name = "Test Scene 3 (shuttle)";

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
        tc->setWorldPosition(shuttlePositions[i]);
        tc->setWorldScale(Vec3(.25));
        
        auto rcc = addRenderComponents(e, shuttleMesh->submeshes.size());

        FOR_COMPONENT_GROUP(rcc)
        {
            RenderComponent *rc = it.ptr;
            new (rc) RenderComponent(e, &(shuttleMesh->submeshes[it.index]));
        }
    }
}

void updateGame(Game* game)
{
    assert(game->activeScene != nullptr);

    //
    // Update camera and position
    //
    walkAndCamera(game);
    
    //
    // Enable/disable editor
    //
    if (keys[GLFW_KEY_GRAVE_ACCENT] && !lastKeys[GLFW_KEY_GRAVE_ACCENT])
    {
        game->editor.isEnabled = !game->editor.isEnabled;

        if (game->editor.isEnabled)
        {
            glfwSetInputMode(game->window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(game->window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    //
    // Render
    //
    {
        CameraComponent* camComponent = getCameraComponent(game->activeCamera);
        TransformComponent* camXfm = getTransformComponent(game->activeCamera);

        renderScene(game->activeScene, camComponent, camXfm->worldTransform());
    }

    //
    // Editor mode
    // Note: Editor should always be shown AFTER rendering
    //
    if (game->editor.isEnabled)
    {
        showEditor(&game->editor);
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
    game->editor.game = game;
    game->editor.isEnabled = false;
    game->window = &window;
    
    Scene* testScene1 = makeScene(game);
    buildTestScene1(testScene1);
    
    Scene* testScene2 = makeScene(game);
    buildTestScene2(testScene2);

    Scene* testScene3 = makeScene(game);
    buildTestScene3(testScene3);
    
    // Set up camera
    // @Hack: camera is owned by test scene 1. Make some entities (player, camera, etc.) independent of
    // scene. Maybe the game has its own ECS outside of scenes or maybe make system to transfer ownership
    // from one scene to another scene
    Entity camera = makeEntity(&testScene1->ecs, "camera");
    TransformComponent* cameraXfm = addTransformComponent(camera);
    CameraComponent* cameraComponent = addCameraComponent(camera);
    cameraComponent->window = &window;

    // Set up player
    // @Hack: same as camera hack
    Entity player = makeEntity(&testScene1->ecs, "player");
    game->player = player;
    {
        TransformComponent* playerXfm = addTransformComponent(player);
        
        ColliderComponent* playerCollider = addColliderComponent(player);
        playerCollider->type = ColliderType::RECT3;
        float32 playerHeight = 2;
        float32 playerWidth = .5;
        playerCollider->rect3Lengths = Vec3(playerWidth, playerHeight, playerWidth);
        playerCollider->xfmOffset = Vec3(0, playerHeight / 2, 0);
        
        WalkComponent* playerWalk = addWalkComponent(player);
        playerWalk->isGrounded = true;
        assert(testScene1->ecs.walkComponents.count() > 0);
        playerWalk->terrain = testScene1->ecs.terrains[0].entity;
    }

    cameraXfm->parent = player;
    
#if 1
    cameraComponent->isOrthographic = false;
    cameraComponent->perspectiveFov = 60.0f;
#else
    cameraComponent->isOrthographic = true;
    cameraComponent->orthoWidth = 10.0f;
#endif
    
    cameraComponent->aspectRatio = windowWidth / (float32)windowHeight;
    cameraComponent->near = 0.01f;
    cameraComponent->far = 1000.0f;
    recalculateProjectionMatrix(cameraComponent);

    DebugDraw::instance().cameraComponent = cameraComponent;
    DebugDraw::instance().cameraXfm = cameraXfm;
    DebugDraw::instance().window = &window;

    ColliderComponent* debugCC;
    
    //
    // Set up portal from scene 1<->2
    //
    {
        Entity portal1 = makeEntity(&testScene1->ecs, "portalA1");
        Entity portal2 = makeEntity(&testScene2->ecs, "portalA2");

        Transform portal1Xfm;
        portal1Xfm.setPosition(Vec3(0, 0, -10));
        portal1Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 180));
        
        Transform portal2Xfm;
        portal2Xfm.setPosition(Vec3(1, 2, 3));
        portal2Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 45));

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
        portal1Xfm.setPosition(Vec3(0, 0, -10));
        portal1Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 0));
        
        Transform portal3Xfm;
        portal3Xfm.setPosition(Vec3(0, 0, 0));
        portal3Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 0));

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
