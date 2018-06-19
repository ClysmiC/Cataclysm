#pragma once

#include "Component.h"
#include "als_math.h"

struct Ray;
struct Window;

struct CameraComponent : public Component
{
	Mat4 projectionMatrix;
	bool isOrthographic;

	real32 aspectRatio;
	real32 near;
	real32 far;

	Window* window;
	
	union
	{
		real32 perspectiveFov;
		real32 orthoWidth;
	};

	// TODO: wrappers around the math library calls
	// initPerspective(..)
	// initOrtho(..)
};

void recalculateProjectionMatrix(CameraComponent* camera);

// "pixel" space: from 0, 0 (bot left) to w - 1, h - 1 (top right)
Ray rayThroughScreenCoordinate(CameraComponent* camera, Vec2 screenCoordinate);

// viewport space: from 0, 0, (bot left) to 1, 1 (top right)
Ray rayThroughViewportCoordinate(CameraComponent* camera, Vec2 viewportCoordinate);

