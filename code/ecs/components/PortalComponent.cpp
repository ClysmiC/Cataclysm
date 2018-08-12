#include "GL/glew.h"

#include "PortalComponent.h"
#include "resource/ResourceManager.h"
#include "Game.h"
#include "ecs/Ecs.h"

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
        xfm->setScale(Vec3(dimensions.x, dimensions.y, xfm->scale().z));
    }
    else assert(false);

    if (propogateToConnectedPortal)
    {
        PortalComponent* connectedPortal = getConnectedPortal(portal);
        setDimensions(connectedPortal, dimensions, false);
    }
}

Vec2 getDimensions(PortalComponent* portal)
{
    TransformComponent* xfm = getTransformComponent(portal->entity);

    assert(xfm != nullptr);
    if (xfm == nullptr) return Vec2(0);
    
    Vec2 result = xfm->scale().xy();

    return result;
}

PortalComponent* getConnectedPortal(PortalComponent* portal)
{
    Entity connectedPortal = getEntity(getGame(portal->entity), &portal->connectedPortal);
    PortalComponent* connectedPortalComponent = getPortalComponent(connectedPortal);

    assert(connectedPortalComponent != nullptr);

    return connectedPortalComponent;
}

Scene* getConnectedScene(PortalComponent* portal)
{
    Entity connectedPortal = getEntity(getGame(portal->entity), &portal->connectedPortal);
    return connectedPortal.ecs->scene;
}

TransformComponent* getConnectedSceneXfm(PortalComponent* portal)
{
    Entity connectedPortal = getEntity(getGame(portal->entity), &portal->connectedPortal);
    TransformComponent* result = getTransformComponent(connectedPortal);
    return result;
}

Vec3 intoPortalNormal(PortalComponent* portal)
{
    TransformComponent* xfm = getTransformComponent(portal->entity);
    Vec3 result = xfm->back();
    return result;
}

Vec3 outOfPortalNormal(PortalComponent* portal)
{
    TransformComponent* xfm = getTransformComponent(portal->entity);
    Vec3 result = xfm->forward();
    return result;
}

Vec3 intoConnectedPortalNormal(PortalComponent* portal)
{
    TransformComponent* xfm = getConnectedSceneXfm(portal);
    Vec3 result = xfm->back();
    return result;
}

Vec3 outOfConnectedPortalNormal(PortalComponent* portal)
{
    TransformComponent* xfm = getConnectedSceneXfm(portal);
    Vec3 result = xfm->forward();
    return result;
}

void rebaseTransformInPlace(PortalComponent* portal, ITransform* transform)
{
    TransformComponent* sourceSceneXfm = getTransformComponent(portal->entity);;
    TransformComponent* connectedSceneXfm = getConnectedSceneXfm(portal);

    Quaternion intoSourcePortal = axisAngle(sourceSceneXfm->up(), 180) * sourceSceneXfm->orientation();
    Quaternion outOfConnectedPortal = connectedSceneXfm->orientation();
    
    Quaternion rotationNeeded = relativeRotation(intoSourcePortal, outOfConnectedPortal);
    
    Vec3 offsetToSourcePortal = sourceSceneXfm->position() - transform->position();
    Vec3 offsetToSourcePortalTransformedToConnectedScene = rotationNeeded * offsetToSourcePortal;

    Vec3 positionInConnectedScene = connectedSceneXfm->position() - offsetToSourcePortalTransformedToConnectedScene; 

    transform->setPosition(positionInConnectedScene);
    transform->setOrientation(rotationNeeded * transform->orientation());
}

void createPortalFromTwoBlankEntities(Entity portal1, Entity portal2, ITransform* portal1Xfm, ITransform* portal2Xfm, Vec2 dimensions)
{
    PortalComponent* portal1Component = addPortalComponent(portal1);
    PortalComponent* portal2Component = addPortalComponent(portal2);
    portal1Component->connectedPortal = portal2;
    portal2Component->connectedPortal = portal1;
    
    TransformComponent* portal1XfmComponent = getTransformComponent(portal1);
    portal1XfmComponent->setPosition(portal1Xfm->position());
    portal1XfmComponent->setOrientation(portal1Xfm->orientation());
    portal1XfmComponent->setScale(portal1Xfm->scale());

    TransformComponent* portal2XfmComponent = getTransformComponent(portal2);
    portal2XfmComponent->setPosition(portal2Xfm->position());
    portal2XfmComponent->setOrientation(portal2Xfm->orientation());
    portal2XfmComponent->setScale(portal2Xfm->scale());

    // The collider is a box "behind" the portal (think of the loading zone behind a painting in sm64, where the painting is the portal)
    float32 colliderDepth = 1.0f;
    
    ColliderComponent* portal1Collider = addColliderComponent(portal1);
    portal1Collider->xfmOffset = Vec3(Axis3D::Z) * colliderDepth / 2;
    portal1Collider->rect3Lengths.x = 1;
    portal1Collider->rect3Lengths.y = 1;
    portal1Collider->rect3Lengths.z = colliderDepth;
    portal1Collider->type = ColliderType::RECT3;

    ColliderComponent* portal2Collider = addColliderComponent(portal2);
    portal2Collider->xfmOffset = Vec3(Axis3D::Z) * colliderDepth / 2;
    portal2Collider->rect3Lengths.x = 1;
    portal2Collider->rect3Lengths.y = 1;
    portal2Collider->rect3Lengths.z = colliderDepth;
    portal2Collider->type = ColliderType::RECT3;

    setDimensions(portal1Component, dimensions, true);
}
