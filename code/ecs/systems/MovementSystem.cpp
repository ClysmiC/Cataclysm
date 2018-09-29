#include "MovementSystem.h"

#include "als/als_types.h"

#include "Game.h"
#include "ecs/Ecs.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"

#include "ecs/components/ColliderComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/TerrainComponent.h"
#include "ecs/components/WalkComponent.h"

#include "Gjk.h"

void walkAndCamera(Game* game)
{
    TransformComponent* xfm = getTransformComponent(game->player);
    TransformComponent* cameraXfm = getTransformComponent(game->activeCamera);
    WalkComponent* walk = getWalkComponent(game->player);
    ColliderComponent* collider = getColliderComponent(game->player);
    EntityDetails* details = getEntityDetails(game->player);

    assert((details->flags & EntityFlag_Static) == 0); // only dynamic object can walk!
    
    TerrainComponent *terrain = getTerrainComponent(walk->terrain);
    
    Vec3 posBeforeMove = xfm->position();
    
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

    bool draggingCameraInEditMode =
        game->editor.isEnabled &&
        !game->editor.translator.isHandleSelected &&
        mouseButtons[GLFW_MOUSE_BUTTON_1] &&
        !ImGui::GetIO().WantCaptureMouse;

    if (!game->editor.isEnabled || draggingCameraInEditMode)
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

            Quaternion deltaYaw;
            Quaternion deltaPitch;
            deltaYaw = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * -deltaMouseX * deltaTS); // yaw
            deltaPitch = axisAngle(Vec3(1, 0, 0), cameraTurnSpeed * deltaMouseY * deltaTS); // pitch

            // "Player" just yaws.
            // Camera yaws and pitches
            xfm->setOrientation(deltaYaw * xfm->orientation());

            cameraXfm->setLocalOrientation(deltaPitch * cameraXfm->localOrientation());
        }

        if (keys[GLFW_KEY_W])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveForward * cameraSpeed * deltaTS);
        }
        else if (keys[GLFW_KEY_S])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveBack * cameraSpeed * deltaTS);
        }
        
        if (keys[GLFW_KEY_A])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveLeft * cameraSpeed * deltaTS);
        }
        else if (keys[GLFW_KEY_D])
        {
            xfm->setLocalPosition(xfm->localPosition() + moveRight * cameraSpeed * deltaTS);
        }
    }

    //
    // Resolve collision
    //
    if (collider)
    {
        FOR_BUCKET_ARRAY (game->activeScene->ecs.colliders.components)
        {
            // @Slow. Improve with spatial partitioning
            ColliderComponent* cc = it.ptr;
            EntityDetails* debug_details = getEntityDetails(cc->entity);

            if (cc->isTrigger || cc->entity.id == walk->entity.id) continue;

            GjkResult collisionResult = gjk(collider, cc);

            if (collisionResult.collides)
            {
                xfm->setPosition(xfm->position() - collisionResult.penetrationVector);
            }
        }
    }

    //
    // Snap to ground
    //
    if (terrain)
    {
        float32 height = getTerrainHeight(terrain, xfm->position().x, xfm->position().z);

        xfm->setPosition(
            Vec3(
                xfm->position().x,
                height,
                xfm->position().z
            )
        );
    }

    //
    // Go thru portal
    //
    FOR_BUCKET_ARRAY (game->activeScene->ecs.portals.components)
    {
        PortalComponent* pc = it.ptr;
        if (pc->connectedPortal.id == 0) continue;
        
        ColliderComponent* cc = getColliderComponent(pc->entity);

        if (pointInsideCollider(cc, xfm->position()))
        {
            Vec3 portalPos = getTransformComponent(pc->entity)->position();
            Vec3 portalToOldPos = posBeforeMove - portalPos;
            Vec3 portalToPos = xfm->position() - portalPos;
            
            if (dot(portalToOldPos, outOfPortalNormal(pc)) >= 0)
            {
                rebaseTransformInPlace(pc, xfm);
                EntityDetails* connectedPortal = getEntityDetails(getEntity(getGame(pc->entity), &pc->connectedPortal));
                game->activeScene = connectedPortal->entity.ecs->scene;
            }
        }
    }
}
