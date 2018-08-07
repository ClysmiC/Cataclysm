#pragma once

#include "Component.h"
#include "als_math.h"
#include "TransformComponent.h"

struct Scene;
struct Shader;
struct Scene;

struct PortalComponent : public Component
{
    PortalComponent() = default;
    uint32 connectedPortalEntityId;
    
    static constexpr bool multipleAllowedPerEntity = false;
};

Shader* portalShader();

void setDimensions(PortalComponent* portal, Vec2 dimensions, bool propogateToConnectedPortal=true);
Vec2 getDimensions(PortalComponent* portal);

PortalComponent* getConnectedPortal(PortalComponent* portal);
Scene* getConnectedScene(PortalComponent* portal);
TransformComponent* getConnectedSceneXfm(PortalComponent* portal);

Vec3 intoPortalNormal(PortalComponent* portal);
Vec3 outOfPortalNormal(PortalComponent* portal);
Vec3 intoConnectedPortalNormal(PortalComponent* portal);
Vec3 outOfConnectedPortalNormal(PortalComponent* portal);
void rebaseTransformInPlace(PortalComponent* portal, ITransform* transform);

void createPortalFromTwoBlankEntities(Entity portal1, Entity portal2, ITransform* portal1Xfm, ITransform* portal2Xfm, Vec2 dimensions);

