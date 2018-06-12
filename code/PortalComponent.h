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
	
	Scene* scene1;
	Scene* scene2;

	// TODO: turn this into just a "transform" struct.
	// The "transform" component will just be a wrapper around the transform struct
	Transform scene1Xfm;
	Transform scene2Xfm;
};

Shader* portalShader();

void setDimensions(PortalComponent* portal, Vec2 dimensions);
Vec2 getDimensions(PortalComponent* portal);

Scene* getDestScene(PortalComponent* portal, Scene* sourceScene);
Transform* getSourceSceneXfm(PortalComponent* portal, Scene* sourceScene);
Transform* getDestSceneXfm(PortalComponent* portal, Scene* sourceScene);

Vec3 intoSourcePortalNormal(PortalComponent* portal, Scene* sourceScene);
Vec3 outOfSourcePortalNormal(PortalComponent* portal, Scene* sourceScene);
Vec3 intoDestPortalNormal(PortalComponent* portal, Scene* sourceScene);
Vec3 outOfDestPortalNormal(PortalComponent* portal, Scene* sourceScene);
void rebaseTransformInPlace(PortalComponent* portal, Scene* sourceScene, Transform* transform);

