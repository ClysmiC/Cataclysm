#include "MovementSystem.h"

#include "als/als_types.h"

#include "Game.h"
#include "ecs/Ecs.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"

#include "ecs/components/ConvexHullColliderComponent.h"
#include "ecs/components/ColliderComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/TerrainComponent.h"
#include "ecs/components/AgentComponent.h"
#include "ecs/components/WalkComponent.h"

#include "Gjk.h"

void walkAndCamera(Game* game)
{
    TransformComponent* xfm = getTransformComponent(game->player);
    ColliderComponent* collider = getColliderComponent(game->player);
    EntityDetails* details = getEntityDetails(game->player);
    AgentComponent* agent = getAgentComponent(game->player);

    assert((details->flags & EntityFlag_Static) == 0); // only dynamic object can walk!

    TransformComponent* camXfm = getTransformComponent(game->activeCamera);

    float32 deltaTS = deltaTMs / 1000.0f;

    //Vec3 posBeforeMove = xfm->position();
    
    Plane movementPlane(Vec3(0, 0, 0), Vec3(0, 1, 0));

    Vec3 moveRight   = normalize(project(camXfm->right(), movementPlane));
    Vec3 moveLeft    = -moveRight;
    Vec3 moveForward = normalize(project(camXfm->forward(), movementPlane));
    Vec3 moveBack    = -moveForward;

    assert(FLOAT_EQ(moveRight.y, 0, 0.001));
    assert(FLOAT_EQ(moveForward.y, 0, 0.001));

    const float32 stickDeadzone = 0.05;
    const float32 playerAccel = 35;
    const float32 friction = 4;
    const float gravity = 9.8;

    Vec2 movementInput = moveRight.xz() * (abs(leftJoyX) >= stickDeadzone ? leftJoyX : 0) + 
                    moveForward.xz() * (abs(leftJoyY) >= stickDeadzone ? leftJoyY : 0);

    if (length(movementInput) > 1) movementInput.normalizeInPlace();

    Vec2 acceleration = playerAccel * movementInput;
    acceleration -= friction * agent->velocity;

    Vec3 accelVec3 = Vec3(acceleration.x, 0, acceleration.y);
    Vec3 velocityVec3 = Vec3(agent->velocity.x, 0, agent->velocity.y);
    xfm->setPosition(xfm->position() + 0.5 * accelVec3 * deltaTS * deltaTS + velocityVec3 * deltaTS);

    float32 yaw = TO_DEG(atan2(-agent->velocity.y, agent->velocity.x));
    xfm->setOrientation(axisAngle(Vec3(0, 1, 0), yaw - 90));

    agent->velocity += acceleration * deltaTS;

    // float32 playerSpeed = length(physics->velocity);
    //if (playerSpeed > maxPlayerSpeed)
    //{
    //    physics->velocity /= playerSpeed;    // normalize
    //    physics->velocity *= maxPlayerSpeed; // set to max
    //}

    // Friction
    //physics->velocity -= normalizeOrZero(physics->velocity) * friction * deltaTS;
    

    //bool draggingCameraInEditMode =
    //    game->editor.isEnabled &&
    //    !game->editor.translator.isHandleSelected &&
    //    mouseButtons[GLFW_MOUSE_BUTTON_1] &&
    //    !ImGui::GetIO().WantCaptureMouse;

    // TODO: guard this if in editor mode?
    //if (mouseXPrev != FLT_MAX && mouseYPrev != FLT_MAX)
    //{
    //    // Rotate
    //    float32 deltaMouseX = mouseX - mouseXPrev;
    //    float32 deltaMouseY = mouseY - mouseYPrev;

    //    if (draggingCameraInEditMode)
    //    {
    //        // Drag gesture moves in opposite direction
    //        deltaMouseX = -deltaMouseX;
    //        deltaMouseY = -deltaMouseY;
    //    }

    //    Quaternion deltaYaw;
    //    Quaternion deltaPitch;
    //    deltaYaw = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * -deltaMouseX * deltaTS); // yaw
    //    deltaPitch = axisAngle(Vec3(1, 0, 0), cameraTurnSpeed * deltaMouseY * deltaTS); // pitch

    //    // "Player" just yaws.
    //    // Camera yaws and pitches
    //    xfm->setOrientation(deltaYaw * xfm->orientation());
    //}

    //if (keys[GLFW_KEY_W])
    //{
    //    xfm->setLocalPosition(xfm->localPosition() + moveForward * cameraSpeed * deltaTS);
    //}
    //else if (keys[GLFW_KEY_S])
    //{
    //    xfm->setLocalPosition(xfm->localPosition() + moveBack * cameraSpeed * deltaTS);
    //}
    //    
    //if (keys[GLFW_KEY_A])
    //{
    //    xfm->setLocalPosition(xfm->localPosition() + moveLeft * cameraSpeed * deltaTS);
    //}
    //else if (keys[GLFW_KEY_D])
    //{
    //    xfm->setLocalPosition(xfm->localPosition() + moveRight * cameraSpeed * deltaTS);
    //}

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

            if (cc->isTrigger || cc->entity.id == game->player.id) continue;

            GjkResult collisionResult = gjk(collider, cc);

            if (collisionResult.collides)
            {
                xfm->setPosition(xfm->position() - collisionResult.penetrationVector);
                //physics->velocity -= project(physics->velocity, normalize(collisionResult.penetrationVector));
            }
        }

        FOR_BUCKET_ARRAY (game->activeScene->ecs.convexHullColliders.components)
        {
            // @Slow. Improve with spatial partitioning
            ConvexHullColliderComponent* cc = it.ptr;
            EntityDetails* debug_details = getEntityDetails(cc->entity);

            if (cc->isTrigger || cc->entity.id == game->player.id) continue;

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
    //if (terrain)
    //{
    //    float32 height = getTerrainHeight(terrain, xfm->position().x, xfm->position().z);

    //    xfm->setPosition(
    //        Vec3(
    //            xfm->position().x,
    //            height,
    //            xfm->position().z
    //        )
    //    );
    //}

    //
    // Go thru portal
    //
    //FOR_BUCKET_ARRAY (game->activeScene->ecs.portals.components)
    //{
    //    PortalComponent* pc = it.ptr;
    //    if (pc->connectedPortal.id == 0) continue;
    //    
    //    ColliderComponent* cc = getColliderComponent(pc->entity);

    //    if (pointInsideCollider(cc, xfm->position()))
    //    {
    //        Vec3 portalPos = getTransformComponent(pc->entity)->position();
    //        Vec3 portalToOldPos = posBeforeMove - portalPos;
    //        Vec3 portalToPos = xfm->position() - portalPos;
    //        
    //        if (dot(portalToOldPos, outOfPortalNormal(pc)) >= 0)
    //        {
    //            rebaseTransformInPlace(pc, xfm);
    //            EntityDetails* connectedPortal = getEntityDetails(getEntity(getGame(pc->entity), &pc->connectedPortal));
    //            game->activeScene = connectedPortal->entity.ecs->scene;
    //        }
    //    }
    //}
}
