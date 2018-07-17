#pragma once

#include "Ecs.h"
#include "ColliderComponent.h"

struct Game;

struct EditorState
{
    EditorState();
    
    struct TranslatorTool
    {
        bool isLocalXyz = true;
        bool isHandleSelected = false;
        Axis3D selectedHandle = Axis3D::X;

        Entity pseudoEntity;

        union
        {
            struct
            {
                ColliderComponent* xAxisHandle;
                ColliderComponent* yAxisHandle;
                ColliderComponent* zAxisHandle;
            };

            ColliderComponent* handles[3];
        };
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
