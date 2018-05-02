#pragma once

#include "Mesh.h"
#include "Component.h"
#include "TransformComponent.h"

struct RenderComponent : public Component
{
	SubmeshOpenGlInfo ncSubmeshOpenGlInfo;
	TransformComponent ncTransform;

	// TODO: can we copy a non-canonical set of the needed data here?
	Material* material;

	// TODO: I am chasing a pointer here because the camera will be the same for
	// pretty much every rendered component per frame. Investigate if including
	// a camera by value will be better due to cache performance or slower
	// due to all the unnecessary copying
	Camera* camera;

	void draw();
};
