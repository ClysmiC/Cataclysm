#pragma once

#include "Scene.h"

extern float mouseX;
extern float mouseY;
extern float mouseXPrev;
extern float mouseYPrev;

extern bool keys[1024];
extern bool lastKeys[1024];

extern bool mouseButtons[8];
extern bool lastMouseButtons[8];

#define MAX_SCENES 8

struct Window;

struct EditorState
{
    Entity selectedEntity;
};

struct Game
{
    Game() = default;

    // Any component confined to a scene in this array's ECS.
    // Only 1 (or maybe a few) of these scenes will be active at a time
    Scene scenes[MAX_SCENES];

    EditorState editor;

    Scene* activeScene;
    Window* window;
    Entity activeCamera;

    int numScenes;

    //
    // Editor stuff
    //
    bool isEditorMode;
    bool drawAabb;
    bool drawCollider;
};

Scene* makeScene(Game* game);
void makeSceneActive(Game* game, Scene* scene);
void makeCameraActive(Game* game, Entity camera);

