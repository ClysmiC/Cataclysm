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
	
	Scene *sourceScene;
	Scene *destScene;
	TransformComponent sourceSceneXfm;
	TransformComponent destSceneXfm;
};

Shader* portalShader();

void setDimensions(PortalComponent* portal, Vec2 dimensions);
Vec2 getDimensions(PortalComponent* portal);
