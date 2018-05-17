#pragma once

#include "Mesh.h"
#include "Component.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

struct RenderComponent : public Component
{
    void init(const Submesh &submesh);

	SubmeshOpenGlInfo submeshOpenGlInfo;

	// TODO: can we copy a non-canonical set of the needed data here?
	Material* material;

	void draw(TransformComponent* xfm, CameraComponent* camera);
};
