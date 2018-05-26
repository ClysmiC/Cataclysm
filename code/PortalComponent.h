#pragma once

#include "Component.h"
#include "als_math.h"

struct Scene;

struct PortalComponent : public Component
{
	PortalComponent();

	Vec2 dimensions;
	Scene *connectedScene;
	CameraEntity cameraIntoConnectedScene;
};
