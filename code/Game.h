#pragma once

#include "Editor.h"
#include "Scene.h"
#include "ecs/systems/RenderSystem.h"

extern float mouseX;
extern float mouseY;
extern float mouseXPrev;
extern float mouseYPrev;

extern float32 timeMs;
extern float32 deltaTMs;

extern bool keys[1024];
extern bool lastKeys[1024];

extern bool mouseButtons[8];
extern bool lastMouseButtons[8];

extern int debug_shadowMapState;

#define MAX_SCENES 8

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
