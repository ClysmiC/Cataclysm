#pragma once

#include "Component.h"

#include "als_math.h"
#include "TransformComponent.h"

struct CameraComponent : public Component
{
	Mat4 projectionMatrix;
	bool isOrthographic;

	// TODO: wrappers around the math library calls
	// initPerspective(..)
	// initOrtho(..)
};

