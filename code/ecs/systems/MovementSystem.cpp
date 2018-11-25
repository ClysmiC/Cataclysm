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

    TransformComponent* camXfm = getTransformComponent(game->activeCamera);
    
    float32 deltaTS = deltaTMs / 1000.0f;

    //
    // Begin lambda
    //
    auto resolveCollisions = [&]()
    {
        bool collided = false;

        if (collider)
        {
            FOR_BUCKET_ARRAY(game->activeScene->ecs.colliders.components)
            {
                // @Slow. Improve with spatial partitioning
                ColliderComponent* cc = it.ptr;
                EntityDetails* debug_details = getEntityDetails(cc->entity);

                if (cc->isTrigger || cc->entity.id == game->player.id) continue;

                GjkResult collisionResult = gjk(collider, cc);

                if (collisionResult.collides)
                {
                    collided = true;
                    xfm->setPosition(xfm->position() - collisionResult.penetrationVector);
                    //physics->velocity -= project(physics->velocity, normalize(collisionResult.penetrationVector));
                }
            }

            FOR_BUCKET_ARRAY(game->activeScene->ecs.convexHullColliders.components)
            {
                // @Slow. Improve with spatial partitioning
                ConvexHullColliderComponent* cc = it.ptr;
                EntityDetails* debug_details = getEntityDetails(cc->entity);

                if (cc->isTrigger || cc->entity.id == game->player.id) continue;

                GjkResult collisionResult = gjk(collider, cc);

                if (collisionResult.collides)
                {
                    collided = true;
                    xfm->setPosition(xfm->position() - collisionResult.penetrationVector);
                }
            }
        }

        return collided;
    };

    //
    // End lambda
    //

    assert((details->flags & EntityFlag_Static) == 0); // only dynamic object can walk!
    
    //
    // Move (w/o gravity)
    //
    {
        Plane movementPlane(Vec3(0, 0, 0), Vec3(0, 1, 0));

        Vec3 moveRight = normalize(project(camXfm->right(), movementPlane));
        Vec3 moveLeft = -moveRight;
        Vec3 moveForward = normalize(project(camXfm->forward(), movementPlane));
        Vec3 moveBack = -moveForward;

        assert(FLOAT_EQ(moveRight.y, 0, 0.001));
        assert(FLOAT_EQ(moveForward.y, 0, 0.001));

        const float32 stickDeadzone = 0.05;
        const float32 playerAccel = 35;
        const float32 friction = 4;

        Vec2 movementInput = moveRight.xz() * (abs(leftJoyX) >= stickDeadzone ? leftJoyX : 0) +
            moveForward.xz() * (abs(leftJoyY) >= stickDeadzone ? leftJoyY : 0);

        if (length(movementInput) > 1) movementInput.normalizeInPlace();

        Vec2 acceleration = playerAccel * movementInput;
        acceleration -= friction * agent->velocity;

        xfm->setPosition(xfm->position() + 0.5 * Vec3(acceleration.x, 0, acceleration.y) * deltaTS * deltaTS + Vec3(agent->velocity.x, 0, agent->velocity.y) * deltaTS);

        resolveCollisions();

        agent->velocity += acceleration * deltaTS;
        float32 yaw = TO_DEG(atan2(-agent->velocity.y, agent->velocity.x));
        xfm->setOrientation(axisAngle(Vec3(0, 1, 0), yaw - 90));
    }

    //
    // Apply gravity
    //
    {
        const float32 gravity = -9.8;
        const float32 drag = 1;

        float32 gravAccel = gravity - drag * agent->yVelocity;
        Vec3 gravAccelVector = Vec3(0, gravAccel, 0);
        Vec3 gravVelVector = Vec3(0, agent->yVelocity, 0);

        xfm->setPosition(xfm->position() + 0.5 * gravAccelVector * deltaTS * deltaTS + gravVelVector * deltaTS);

        if (resolveCollisions())
        {
            agent->isGrounded = true;
            agent->yVelocity = 0;
        }
        else
        {
            agent->isGrounded = false;
            agent->yVelocity += gravAccel * deltaTS;
        }
    }
}
