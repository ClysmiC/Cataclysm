#pragma once

#include "Mesh.h"
#include "Component.h"
#include "TransformComponent.h"

struct RenderComponent : public Component
{
    void init(const Submesh &submesh);

	SubmeshOpenGlInfo submeshOpenGlInfo;

	// TODO: can we copy a non-canonical set of the needed data here?
	Material* material;

	// TODO: I am chasing a pointer here because the camera will be the same for
	// pretty much every rendered component per frame. Investigate if including
	// a camera by value will be better due to cache performance or slower
	// due to all the unnecessary copying
    
    // TODO: should this be part of the struct or passed in as a parameter
	/* Camera* camera; */

	void draw(TransformComponent* xfm, Camera& camera);
};

struct RenderComponentCollection
{
    uint32 numComponents;
    RenderComponent* renderComponents;
};
