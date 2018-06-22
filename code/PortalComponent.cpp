#include "GL/glew.h"

#include "PortalComponent.h"
#include "ResourceManager.h"
#include "Ecs.h"

Shader* portalShader()
{
    static Shader* shader_ = nullptr;

    if (shader_ == nullptr)
    {
        shader_ = ResourceManager::instance().initShader("shader/portal.vert", "shader/portal.frag", true);
    }

    return shader_;
}

void setDimensions(PortalComponent* portal, Vec2 dimensions, bool propogateToConnectedPortal)
{
    TransformComponent* xfm = getTransformComponent(portal->entity);
    if (xfm)
    {
        xfm->scale.x = dimensions.x;
        xfm->scale.y = dimensions.y;
    }
    else assert(false);


    // Don't set the collider dimensions -- these default to 1x1 (as do the portal dimensions)
    // but they are affected by xfm->scale, so they will already scale up
    
    // ColliderComponent* collider = getColliderComponent(portal->entity);
    // if (collider && collider->type == ColliderType::RECT3)
    // {
    //     collider->xLength = dimensions.x;
    //     collider->yLength = dimensions.y;
    // }
    // else
    // {
    //     assert(false);
    // }

    if (propogateToConnectedPortal)
    {
        setDimensions(portal->connectedPortal, dimensions, false);
    }
}

Vec2 getDimensions(PortalComponent* portal)
{
    TransformComponent* xfm = getTransformComponent(portal->entity);

    assert(xfm != nullptr);
    if (xfm == nullptr) return Vec2(0);
    
    Vec2 result = Vec2(xfm->scale.x, xfm->scale.y);

    return result;
}

Scene* getConnectedScene(PortalComponent* portal)
{
    return portal->connectedPortal->entity.ecs->scene;
}

Transform* getConnectedSceneXfm(PortalComponent* portal)
{
    return getTransformComponent(portal->connectedPortal->entity);
}

Vec3 intoPortalNormal(PortalComponent* portal)
{
    Transform* xfm = getTransformComponent(portal->entity);
    Vec3 result = xfm->back();
    return result;
}

Vec3 outOfPortalNormal(PortalComponent* portal)
{
    Transform* xfm = getTransformComponent(portal->entity);
    Vec3 result = xfm->forward();
    return result;
}

Vec3 intoConnectedPortalNormal(PortalComponent* portal)
{
    Transform* xfm = getConnectedSceneXfm(portal);
    Vec3 result = xfm->back();
    return result;
}

Vec3 outOfConnectedPortalNormal(PortalComponent* portal)
{
    Transform* xfm = getConnectedSceneXfm(portal);
    Vec3 result = xfm->forward();
    return result;
}

void rebaseTransformInPlace(PortalComponent* portal, Transform* transform)
{
    Transform* sourceSceneXfm = getTransformComponent(portal->entity);;
    Transform* connectedSceneXfm = getConnectedSceneXfm(portal);

    Quaternion intoSourcePortal = axisAngle(sourceSceneXfm->up(), 180) * sourceSceneXfm->orientation;
    Quaternion outOfConnectedPortal = connectedSceneXfm->orientation;
    
    Quaternion rotationNeeded = relativeRotation(intoSourcePortal, outOfConnectedPortal);
    
    Vec3 offsetToSourcePortal = sourceSceneXfm->position - transform->position;
    Vec3 offsetToSourcePortalTransformedToConnectedScene = rotationNeeded * offsetToSourcePortal;

    Vec3 positionInConnectedScene = connectedSceneXfm->position - offsetToSourcePortalTransformedToConnectedScene; 

    transform->position = positionInConnectedScene;
    transform->orientation = rotationNeeded * transform->orientation;
}

void createPortalFromTwoBlankEntities(Entity portal1, Entity portal2, Transform portal1Xfm, Transform portal2Xfm, Vec2 dimensions)
{
    PortalComponent* portal1Component = addPortalComponent(portal1);
    PortalComponent* portal2Component = addPortalComponent(portal2);
    portal1Component->connectedPortal = portal2Component;
    portal2Component->connectedPortal = portal1Component;
    
    TransformComponent* portal1XfmComponent = addTransformComponent(portal1);
    portal1XfmComponent->position = portal1Xfm.position;
    portal1XfmComponent->orientation = portal1Xfm.orientation;
    portal1XfmComponent->scale = portal1Xfm.scale;

    TransformComponent* portal2XfmComponent = addTransformComponent(portal2);
    portal2XfmComponent->position = portal2Xfm.position;
    portal2XfmComponent->orientation = portal2Xfm.orientation;
    portal2XfmComponent->scale = portal2Xfm.scale;

    // The collider is a box "behind" the portal (think of the loading zone behind a painting in sm64, where the painting is the portal)
    real32 colliderDepth = 1.0f;
    
    ColliderComponent* portal1Collider = addColliderComponent(portal1);
    portal1Collider->xfmOffset = Vec3(Axis3D::Z) * colliderDepth / 2;
    portal1Collider->xLength = 1;
    portal1Collider->yLength = 1;
    portal1Collider->zLength = colliderDepth;
    portal1Collider->type = ColliderType::RECT3;

    ColliderComponent* portal2Collider = addColliderComponent(portal2);
    portal2Collider->xfmOffset = Vec3(Axis3D::Z) * colliderDepth / 2;
    portal2Collider->xLength = 1;
    portal2Collider->yLength = 1;
    portal2Collider->zLength = colliderDepth;
    portal2Collider->type = ColliderType::RECT3;

    setDimensions(portal1Component, dimensions, true);
}
