#pragma once

#include "Mesh.h"
#include "Component.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

struct RenderComponent : public Component
{
	RenderComponent() = default;
	RenderComponent(Entity entity, Submesh* submesh);

	SubmeshOpenGlInfo submeshOpenGlInfo;

	Material* material;
};

void drawRenderComponent(RenderComponent* renderComponent, TransformComponent *xfm,  CameraComponent* camera, TransformComponent *cameraXfm);
