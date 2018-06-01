#pragma once

#include "Component.h"
#include "als_math.h"
#include "TransformComponent.h"

struct Scene;
struct Shader;
struct Scene;

struct PortalComponent : public Component
{
	static real32 quadVertices[];
	static uint32 quadVao();
	static uint32 quadVbo();
	static Shader* shader();
	
	PortalComponent();

	Vec2 getDimensions();
	void setDimensions(Vec2 dim);

	Scene *destScene;
	TransformComponent sourceSceneXfm;
	TransformComponent destSceneXfm;

private:
	Vec2 dimensions;
	static void initQuadVboAndVao();
	static uint32 quadVbo_;
	static uint32 quadVao_;
};
