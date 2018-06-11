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
	TransformComponent scene1Xfm;
	TransformComponent scene2Xfm;
};

Shader* portalShader();

void setDimensions(PortalComponent* portal, Vec2 dimensions);
Vec2 getDimensions(PortalComponent* portal);

Scene* getDestScene(PortalComponent* portal, Scene* sourceScene);
TransformComponent* getSourceSceneXfm(PortalComponent* portal, Scene* sourceScene);
TransformComponent* getDestSceneXfm(PortalComponent* portal, Scene* sourceScene);
