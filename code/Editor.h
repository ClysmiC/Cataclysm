#pragma once

#include "Ecs.h"
#include "ColliderComponent.h"

struct Game;

struct EditorState
{
    EditorState();
    
    bool isLocalXfm = true;
    
    struct TranslatorTool
    {
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

    struct EntityList
    {
        Entity dragging;    // Entity we clicked and started dragging
    };
    
    Game* game;
    bool isEnabled = false;
    Entity selectedEntity;

    Ecs pseudoEcs;
    
    TranslatorTool translator;
    EntityList entityList;
    
    bool drawAabb = true;
    bool drawCollider = true;
};

void showEditor(EditorState* editor);
