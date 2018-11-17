#pragma once

#include "Editor.h"
#include "Scene.h"
#include "ecs/systems/RenderSystem.h"

#define MAX_SCENES 8
#define MOUSE_BUTTON_COUNT 8
#define JOYSTICK_BUTTON_COUNT 32
#define KEY_COUNT 1024

extern float mouseX;
extern float mouseY;
extern float mouseXPrev;
extern float mouseYPrev;

extern float32 timeMs;
extern float32 deltaTMs;

extern bool keys[KEY_COUNT];
extern bool lastKeys[KEY_COUNT];

extern bool joystickButtons[JOYSTICK_BUTTON_COUNT];
extern bool lastJoystickButtons[JOYSTICK_BUTTON_COUNT];

extern float32 leftJoyX;
extern float32 leftJoyY;
extern float32 rightJoyX;
extern float32 rightJoyY;

extern bool mouseButtons[MOUSE_BUTTON_COUNT];
extern bool lastMouseButtons[MOUSE_BUTTON_COUNT];

// TODO: put this away in controller specific file
#define XBOX_GLFW_BUTTON_A                 0
#define XBOX_GLFW_BUTTON_B                 1
#define XBOX_GLFW_BUTTON_X                 2
#define XBOX_GLFW_BUTTON_Y                 3
#define XBOX_GLFW_BUTTON_LB                4
#define XBOX_GLFW_BUTTON_RB                5
#define XBOX_GLFW_BUTTON_SELECT            6
#define XBOX_GLFW_BUTTON_START             7
#define XBOX_GLFW_BUTTON_LEFT_JOY_CLICK    8
#define XBOX_GLFW_BUTTON_RIGHT_JOY_CLICK   9
#define XBOX_GLFW_BUTTON_DPAD_UP          10
#define XBOX_GLFW_BUTTON_DPAD_RIGHT       11
#define XBOX_GLFW_BUTTON_DPAD_DOWN        12
#define XBOX_GLFW_BUTTON_DPAD_LEFT        13

struct Window;

struct Game
{
    // Any component confined to a scene in this array's ECS.
    // Only 1 (or maybe a few) of these scenes will be active at a time
    Scene scenes[MAX_SCENES];

    EditorState editor;

    Scene* activeScene;
    Window* window;

    Renderer renderer;

    // @TODO: Should these be PotentiallyStaleEntity ?
    Entity activeCamera;
    Entity player;

    uint32 numScenes;

    // End of frame bookkeeping
    std::vector<PotentiallyStaleEntity> entitiesMarkedForDeletion;
};

Scene* makeScene(Game* game);
void makeSceneActive(Game* game, Scene* scene);
void makeCameraActive(Game* game, Entity camera);

EntityDetails* getEntityDetails(Game* game, uint32 entityId);
Entity getEntity(Game* game, PotentiallyStaleEntity* potentiallyStaleEntity);
Entity getEntity(Game* game, uint32 entityId);

Game* getGame(Entity e);

void deleteMarkedEntities(Game* game);
