#pragma once

#include "Ecs.h"
#include "ColliderComponent.h"

struct Game;

struct EditorState
{
    EditorState();
    
    struct TranslatorTool
    {        
        int32 selectedHandle = -1;

        Entity pseudoEntity;
        
        ColliderComponent* xAxisHandle;
        ColliderComponent* yAxisHandle;
        ColliderComponent* zAxisHandle;
    };
    
    Game* game;
    bool isEnabled = false;
    Entity selectedEntity;

    Ecs pseudoEcs;
    
    TranslatorTool translator;
    
    bool drawAabb = true;
    bool drawCollider = true;
};

void showEditor(EditorState* editor);
