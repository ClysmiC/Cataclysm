#pragma once

#include "ecs/Ecs.h"

struct Game;
struct ColliderComponent;

struct EditorState
{
    EditorState();
    
    
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

    struct EntityListUi
    {
        static const char* ADD_ENTITY_POPUP_ID;
        static const char* DRAG_DROP_ID;
    };

    struct ComponentListUi
    {
        // Note: these only get set for IDs that we care about tracking,
        //       so they might not always be accurate
        uint32 lastFrameActiveId;
        uint32 thisFrameActiveId;
        
        // User may edit in "non-canonical" Tait-Bryan angles such that the conversion
        // to them from quaternion will give a different (but identical) rotation.
        // Store the user-entered rotation until focus leaves the "hot" field and then
        Vec3 hotXfmEuler;
        
        Vec3 hotNormalizedVec3;

        static const char* ADD_COMPONENT_POPUP_ID;
    };

    //
    // Book-keeping
    //
    Game* game;
    bool isEnabled = false;
    Entity selectedEntity; // @Think: Should this be PotentiallyStaleEntity?
    Ecs pseudoEcs;

    //
    // Editor components
    //
    TranslatorTool  translator;
    EntityListUi    entityList;
    ComponentListUi componentList;

    //
    // Flags (more book-keeping)
    //
    bool isLocalXfm = true;
    bool drawAabb = true;
    bool drawCollider = true;
};

void showEditor(EditorState* editor);
