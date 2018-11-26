// Note: this file is a complete dumping grounds of implementations until I decide how I want to factor them out.
// Will eventually clean it up...

#include "Game.h"

#include "als/als_types.h"
#include "als/als_fixed_string.h"
#include "als/als_bucket_array.h"
#include "als/als_temp_alloc.h"

#include "Window.h"

#include <iostream>
#include <thread>

#include "ecs/Ecs.h"
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

#include "resource/ResourceManager.h"
#include "Scene.h"
#include "DebugDraw.h"
#include "resource/resources/Mesh.h"
#include "GL/glew.h"
#include "Ray.h"
#include "Gjk.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Reflection.h"

#include "ecs/systems/MovementSystem.h"

#include "platform/platform.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Quickhull.h"
#include "resource/ResourceManager.h"

#include "ObjLoading.h"


bool keys[1024];
bool lastKeys[1024];

bool mouseButtons[8];
bool lastMouseButtons[8];

float32 leftJoyX;
float32 leftJoyY;
float32 rightJoyX;
float32 rightJoyY;

bool joystickButtons[32];
bool lastJoystickButtons[32];

float mouseX;
float mouseY;
float mouseXPrev;
float mouseYPrev;

float32 timeMs;
float32 deltaTMs;

void loadLevel(Scene* scene, FilenameString levelFilename)
{
    FilenameString fullFilename = ResourceManager::instance().toFullPath(levelFilename);
    assert(fullFilename.substring(fullFilename.length - 4) == ".obj");

    loadObjSubobjectsAsEntities(levelFilename, &scene->ecs, true, true);
}

EntityDetails* getEntityDetails(Game* game, uint32 entityId)
{
    //
    // @Slow
    // Can we accelerate this somehow? It is very convenient to be able to get the entire entity just from its id
    // (so that way we can only store the ID of "linked" entities (like portals) and not worry about having things
    // like the entity's ecs go out of date (if the entity were to, say, enter a new scene)
    //
    
    EntityDetails* result = nullptr;
    
    for (uint32 i = 0; i < game->numScenes; i++)
    {
        Entity probe;
        probe.id = entityId;
        probe.ecs = &game->scenes[i].ecs;

        result = getEntityDetails(probe);

        if (result) break;
    }

    return result;
}

Entity getEntity(Game* game, uint32 entityId)
{
    // @Slow
    // Can we accelerate this somehow? It is very convenient to be able to get the entire entity just from its id
    // (so that way we can only store the ID of "linked" entities (like portals) and not worry about having things
    // like the entity's ecs go out of date (if the entity were to, say, enter a new scene)
    Entity result;
    EntityDetails* details = getEntityDetails(game, entityId);

    if (details)
    {
        result = details->entity;
    }

    return result;
}

Entity getEntity(Game* game, PotentiallyStaleEntity* potentiallyStaleEntity)
{
    Entity result; potentiallyStaleEntity;
    result.id = potentiallyStaleEntity->id;
    result.ecs = potentiallyStaleEntity->ecs;

    if (potentiallyStaleEntity->id == 0) return result;

    if (result.ecs == nullptr || getEntityDetails(result) == nullptr)
    {
        // Stale!
        result = getEntity(game, potentiallyStaleEntity->id);
        potentiallyStaleEntity->ecs = result.ecs; // Update the stale entity so it won't be stale next time!
    }

    return result;
}

Game* getGame(Entity e)
{
    return e.ecs->game;
}

void deleteMarkedEntities(Game* game)
{
    for (PotentiallyStaleEntity staleEntity : game->entitiesMarkedForDeletion)
    {
        Entity e = getEntity(game, &staleEntity);
        EntityDetails* details = getEntityDetails(e);
        if (details == nullptr)
        {
            assert(false);
            continue;
        }

        TransformComponent*        xfm              = getTransformComponent(e);
        CameraComponent*           camera           = getCameraComponent(e);
        DirectionalLightComponent* directionalLight = getDirectionalLightComponent(e);
        TerrainComponent*          terrain          = getTerrainComponent(e);
        auto                       pointLights      = getPointLightComponents(e);
        auto                       renderComponents = getRenderComponents(e);
        PortalComponent*           portal           = getPortalComponent(e);
        auto                       colliders        = getColliderComponents(e);
        WalkComponent*             walk             = getWalkComponent(e);

        Ecs* ecs = e.ecs;

        // Remove mandatory components
        {
            removeComponent(&ecs->entityDetails, details);
            removeComponent(&ecs->transforms,    xfm);
        }

        //
        // @Note: For the most part, we could directly call templated removeComponent function and just pass the correct collection,
        //        but some components have some logic when they get removed. For example, portal components will unlink their
        //        connected portal (if any) when they are removed.
        //
        //        We do call the templated version for the mandatory components simply because they are so few and we know none of
        //        them require special logic. This isn't necessarily so however, so it could be changed.
        //
    
        // Remove optional single components
        {
            if (camera)           removeCameraComponent(&camera);
            if (directionalLight) removeDirectionalLightComponent(&directionalLight);
            if (terrain)          removeTerrainComponent(&terrain);
            if (portal)           removePortalComponent(&portal);
            if (walk)             removeWalkComponent(&walk);
        }

        // Remove optional multi components
        {
            if (pointLights.numComponents > 0)
            {
                for (uint32 i = 0; i < pointLights.numComponents; i++)
                {
                    PointLightComponent* component = &pointLights[i];
                    removePointLightComponent(&component);
                }
            }

            if (renderComponents.numComponents > 0)
            {
                for (uint32 i = 0; i < renderComponents.numComponents; i++)
                {
                    RenderComponent* component = &renderComponents[i];
                    removeRenderComponent(&component);
                }
            }

            if (colliders.numComponents > 0)
            {
                for (uint32 i = 0; i < colliders.numComponents; i++)
                {
                    ColliderComponent* component = &colliders[i];
                    removeColliderComponent(&component);
                }
            }
        }

        e.ecs->entities.erase(std::find(e.ecs->entities.begin(), e.ecs->entities.end(), e));

        if (game->editor.selectedEntity.id == e.id)
        {
            game->editor.selectedEntity.id = 0;
        }
    }


    game->entitiesMarkedForDeletion.clear();
}

///////////////////////////////////////////////////////////////////////
//////////// BEGIN SCRATCHPAD (throwaway or refactorable code)

void debug_testBucketArray()
{
    BucketArray<int, 16> testArray;

    for (int i = 0; i < 56; i++)
    {
        BucketLocator location = testArray.add(i);

        if (i < 16)
        {
            assert(location.bucketIndex == 0);
            assert(location.slotIndex == i);

            assert(testArray.buckets.size() == 1);

            if (i < 15)
            {
                assert(testArray.unfullBuckets.size() == 1);
            }
            else
            {
                assert(testArray.unfullBuckets.size() == 0);

                BucketLocator toRemove = BucketLocator(0, 13);
                testArray.remove(toRemove);
                assert(testArray.unfullBuckets.size() == 1);

                testArray.add(100);
                assert(testArray.unfullBuckets.size() == 0);
                assert(testArray.buckets.size() == 1);
            }
        }
    }

    FOR_BUCKET_ARRAY(testArray)
    {
        if (it.index == 13) assert(*it.ptr == 100);
        else assert(*it.ptr == (int)it.index);
    }

    // Check the remaining stuff in debugger, too lazy to write asserts :)
    BucketLocator toRemove = BucketLocator(1, 13);
    testArray.remove(toRemove);
    testArray.add(100);

    for (int i = 0; i < 16; i++)
    {
        BucketLocator locator = BucketLocator(0, i);
        testArray.remove(locator);
    }

    for (int i = 0; i < 8; i++)
    {
        BucketLocator locator = BucketLocator(1, i);
        testArray.remove(locator);
    }

    for (int i = 0; i < 24; i++)
    {
        testArray.add(-i);
    }
}

void updateJoystickInput()
{
    if (glfwJoystickPresent(GLFW_JOYSTICK_1))
    {
        int count;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

        if (count >= 2)
        {
            leftJoyX = axes[0];
            leftJoyY = axes[1];
        }

        if (count >= 4)
        {
            rightJoyX = axes[2];
            rightJoyY = axes[3];
        }

        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
        for (int i = 0; i < min(JOYSTICK_BUTTON_COUNT, count); i++)
        {
            joystickButtons[i] = (buttons[i] == GLFW_PRESS);
        }
    }
    else
    {
        leftJoyX = 0;
        leftJoyY = 0;
        rightJoyX = 0;
        rightJoyY = 0;

        for (int i = 0; i < JOYSTICK_BUTTON_COUNT; i++)
        {
            joystickButtons[i] = false;
        }
    }
}

void updateLastKeysAndMouseButtonsAndJoystickButtons()
{
    for (uint32 i = 0; i < KEY_COUNT; i++)
    {
        lastKeys[i] = keys[i];
    }

    for (uint32 i = 0; i < MOUSE_BUTTON_COUNT; i++)
    {
        lastMouseButtons[i] = mouseButtons[i];
    }

    for (uint32 i = 0; i < JOYSTICK_BUTTON_COUNT; i++)
    {
        lastJoystickButtons[i] = joystickButtons[i];
    }
}

void buildTestScene1(Scene* scene)
{
    scene->name = "Test Scene 1 (hex)";

    ResourceManager& rm = ResourceManager::instance();
    
    Mesh *hexMesh = rm.initMesh("hex/hex.obj", true, MeshLoadOptions::CPU_AND_GPU);
    Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, MeshLoadOptions::CPU_AND_GPU);

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
    //const uint32 hexCount = 2;
    //Vec3 hexPositions[hexCount];
    //hexPositions[0] = Vec3(-18, 0, -40);
    //hexPositions[1] = Vec3(6, 0, -4);

    //for (uint32 i = 0; i < hexCount; i++)
    //{
    //    Entity e = makeEntity(&scene->ecs, "hex");
    //    
    //    TransformComponent *tc = getTransformComponent(e);
    //    ColliderComponent* cc = addColliderComponent(e);

    //    if (i == 0)
    //    {
    //        // add some more collider components to test having multiple components on an entity
    //        addColliderComponent(e);
    //        addColliderComponent(e);
    //    }
    //    tc->setPosition(hexPositions[i]);
    //    tc->setScale(Vec3(1));
    //    
    //    auto rcc = addRenderComponents(e, hexMesh->submeshes.size());

    //    for(uint32 j = 0; j < hexMesh->submeshes.size(); j++)
    //    {
    //        RenderComponent *rc = &rcc[j];
    //        new (rc) RenderComponent(e, &(hexMesh->submeshes[j]));
    //    }
    //}

    //
    // Set up terrain
    //
    //{
    //    Entity e = makeEntity(&scene->ecs, "terrain");

    //    // TODO: rendering code expects entities with rendercomponents to also have transforms.
    //    //       we could assume the origin at this case, or maybe set a flag on the entity that explicitly
    //    //       says we don't store a transform so we can have the robustness of that check without the memory
    //    //       overhead
    //    TransformComponent *xfm = getTransformComponent(e);
    //    xfm->setPosition(Vec3(0, 0, 0));
    //    
    //    TerrainComponent* tc = addTerrainComponent(e);
    //    new (tc) TerrainComponent(e, "heightmap.bmp", Vec3(-200, 0, -200), 400, 400, -10, 8);

    //    uint32 numChunks = tc->xChunkCount * tc->zChunkCount;

    //    auto rcList = addRenderComponents(e, numChunks);

    //    for (uint32 i = 0; i < tc->zChunkCount; i++)
    //    {
    //        for (uint32 j = 0; j < tc->xChunkCount; j++)
    //        {
    //            RenderComponent* rc = &rcList[i * tc->xChunkCount + j];

    //            TerrainChunk* chunk = &tc->chunks[i][j];
    //            new (rc) RenderComponent(e, &chunk->mesh.submeshes[0]);
    //        }
    //    }
    //}

    loadLevel(scene, "test_level.obj");
}

void buildTestScene2(Scene* scene)
{
    scene->name = "Test Scene 2 (ico)";

    ResourceManager& rm = ResourceManager::instance();
    
    Mesh *icosahedronMesh = rm.initMesh("icosahedron/icosahedron.obj", true, MeshLoadOptions::CPU_AND_GPU);

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
        TransformComponent *tc = getTransformComponent(e);
        tc->setPosition(icosahedronPositions[i]);

        auto rcc = addRenderComponents(e, icosahedronMesh->submeshes.size());

        ColliderComponent *cc = addColliderComponent(e);
        new (cc) ColliderComponent(e, icosahedronMesh->bounds);
                                   
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
    
    Mesh *shuttleMesh = rm.initMesh("shuttle/shuttle.obj", true, MeshLoadOptions::CPU_AND_GPU);
    Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, MeshLoadOptions::CPU_AND_GPU);

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
        TransformComponent *tc = getTransformComponent(e);
        tc->setPosition(shuttlePositions[i]);
        tc->setScale(Vec3(.25));
        
        auto rcc = addRenderComponents(e, shuttleMesh->submeshes.size());

        auto it = (rcc).bucketArray->addressOf((rcc).components[0]);

        for (uint32 j = 0; j < rcc.numComponents; j++)
        {
            RenderComponent *rc = &rcc[j];
            new (rc) RenderComponent(e, &(shuttleMesh->submeshes[j]));
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
    // Update camera
    //
    {
        Vec3 playerPos = getTransformComponent(game->player)->position();
        Vec3 camPos = playerPos + Vec3(0, 12, 20);
        Quaternion camRot = lookRotation(playerPos - camPos, Vec3(0, 1, 0));

        TransformComponent* camXfm = getTransformComponent(game->activeCamera);
        camXfm->setPosition(camPos);
        camXfm->setOrientation(camRot);
    }
    
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

        renderScene(&game->renderer, game->activeScene, camComponent, camXfm);
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
    result->ecs.game = game;
    
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
    initializeTempAlloc(1024 * 1024); // 1 MiB

    // INIT WINDOW
    uint32 windowWidth = 1600;
    uint32 windowHeight = 900;
    
    Window window;

#if 0
    debug_testBucketArray();
#endif

    if (!initGlfwWindow(&window, windowWidth, windowHeight))
    {
        return -1;
    }

#if 1
    // Test platform layer stuff
    auto result = getAllFileNames("X:/resources", true, "obj");
#endif

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO* io = &ImGui::GetIO();
    io->IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow, true);
    ImGui_ImplOpenGL3_Init();
    
    ImGui::StyleColorsDark();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(.26, .59, .98, .31));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(.26, .59, .98, .8));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(.26, .59, .98, 1));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    ResourceManager::instance().init();
    DebugDraw::instance().init();
    
    float32 lastTimeMs = 0;

    Game* game = new Game();
    
    game->editor.game = game;
    game->editor.pseudoEcs.game = game;
    game->editor.isEnabled = false;
    game->window = &window;
    
    initRenderer(&game->renderer, game->window);
    
    Scene* testScene1 = makeScene(game);
    buildTestScene1(testScene1);
    
    //Scene* testScene2 = makeScene(game);
    //buildTestScene2(testScene2);

    //Scene* testScene3 = makeScene(game);
    //buildTestScene3(testScene3);
    //
    // Set up camera
    // @Hack: camera is owned by test scene 1. Make some entities (player, camera, etc.) independent of
    // scene. Maybe the game has its own ECS outside of scenes or maybe make system to transfer ownership
    // from one scene to another scene
    Entity camera = makeEntity(&testScene1->ecs, "camera");
    TransformComponent* cameraXfm = getTransformComponent(camera);
    CameraComponent* cameraComponent = addCameraComponent(camera);
    cameraComponent->window = &window;

    // Set up player
    // @Hack: same as camera hack
    Entity player = makeEntity(&testScene1->ecs, "player", EntityFlag_None);
    game->player = player;
    {
        TransformComponent* playerXfm = getTransformComponent(player);
        playerXfm->setPosition(Vec3(0, 1, 0)); // @Hack: start "above" ground and fall onto it so I don't have to worry about precisely lining stuff up
        Mesh* marioMesh = ResourceManager::instance().initMesh("sm64/mario.obj", true, MeshLoadOptions::CPU_AND_GPU);

        for (uint32 i = 0; i < marioMesh->submeshes.size(); i++)
        {
            RenderComponent* rc = addRenderComponent(game->player);
            new (rc) RenderComponent(game->player, &marioMesh->submeshes[i]);
        }

        ColliderComponent* playerCollider = addColliderComponent(player);
        playerCollider->type = ColliderType::RECT3;
        float32 playerHeight = 2;
        float32 playerWidth = .5;
        playerCollider->rect3Lengths = Vec3(playerWidth, playerHeight, playerWidth);
        playerCollider->xfmOffset = Vec3(0, playerHeight / 2, 0);

        addAgentComponent(player);
        
        //WalkComponent* playerWalk = addWalkComponent(player);
        //playerWalk->isGrounded = true;
        //assert(testScene1->ecs.walkComponents.count() > 0);
        //playerWalk->terrain = testScene1->ecs.terrains[0].entity;
    }
    
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
    
    ////
    //// Set up portal from scene 1<->2
    ////
    //{
    //    Entity portal1 = makeEntity(&testScene1->ecs, "portalA1");
    //    Entity portal2 = makeEntity(&testScene2->ecs, "portalA2");

    //    Transform portal1Xfm;
    //    portal1Xfm.setPosition(Vec3(0, 0, -10));
    //    portal1Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 180));
    //    
    //    Transform portal2Xfm;
    //    portal2Xfm.setPosition(Vec3(1, 2, 3));
    //    portal2Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 45));

    //    Vec2 dimensions(2, 3);
    //    
    //    createPortalFromTwoBlankEntities(portal1, portal2, &portal1Xfm, &portal2Xfm, dimensions);
    //}

    ////
    //// Set up portal from scene 1<->3
    ////
    //{
    //    Entity portal1 = makeEntity(&testScene1->ecs, "portalB1");
    //    Entity portal3 = makeEntity(&testScene3->ecs, "portalB3");

    //    Transform portal1Xfm;
    //    portal1Xfm.setPosition(Vec3(0, 0, -10));
    //    portal1Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 0));
    //    
    //    Transform portal3Xfm;
    //    portal3Xfm.setPosition(Vec3(0, 0, 0));
    //    portal3Xfm.setOrientation(axisAngle(Vec3(0, 1, 0), 0));

    //    Vec2 dimensions(2, 3);
    //    
    //    createPortalFromTwoBlankEntities(portal1, portal3, &portal1Xfm, &portal3Xfm, dimensions);
    //}

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

        if (deltaTMs > 100) deltaTMs = 100; // prevent huge time skips between frames (such as the first frame which has to load obj models, etc. and can take a second or two)
        lastTimeMs = timeMs;

        updateJoystickInput();

        updateGame(game);
        deleteMarkedEntities(game);

        bool foo;
        ImGui::Begin("Joystick values", &foo);
        ImGui::InputFloat("Joy 0", &leftJoyX);
        ImGui::InputFloat("Joy 1", &leftJoyY);
        ImGui::InputFloat("Joy 2", &rightJoyX);
        ImGui::InputFloat("Joy 3", &rightJoyY);
        ImGui::Separator();

        for (int i = 0; i < 14; i++)
        {
            char buffer[32];
            sprintf_s(buffer, "Button %d", i);
            int value = joystickButtons[i] == true ? 1 : 0;
            ImGui::InputInt(buffer, &value);
        }

        ImGui::End();

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

        updateLastKeysAndMouseButtonsAndJoystickButtons();
        clearTempAlloc();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // TODO: proper FPS and timing stuff
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window.glfwWindow);
    glfwTerminate();
    return 0;
}
