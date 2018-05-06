#pragma once

#include "Entity.h"
#include "als_math.h"

#include "TransformComponent.h"

Mat4 calculateWorldToViewMatrix(TransformComponent *cameraXfm);

