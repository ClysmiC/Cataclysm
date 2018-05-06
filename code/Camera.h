#pragma once

#include "Entity.h"
#include "als_math.h"

#include "TransformComponent.h"

Mat4 calculateWorldToViewMatrix(TransformComponent *cameraXfm);
inline Vec3 cameraLeft(TransformComponent *cameraXfm)
{
	return cameraXfm->orientation() * Vec3(-1, 0, 0);
}

inline Vec3 cameraRight(TransformComponent *cameraXfm)
{
	return cameraXfm->orientation() * Vec3(1, 0, 0);
}

inline Vec3 cameraUp(TransformComponent *cameraXfm)
{
	return cameraXfm->orientation() * Vec3(0, 1, 0);
}

inline Vec3 cameraDown(TransformComponent *cameraXfm)
{
	return cameraXfm->orientation() * Vec3(0, -1, 0);
}

inline Vec3 cameraForward(TransformComponent *cameraXfm)
{
	return cameraXfm->orientation() * Vec3(0, 0, -1);
}

inline Vec3 cameraBack(TransformComponent *cameraXfm)
{
	return cameraXfm->orientation() * Vec3(0, 0, 1);
}


