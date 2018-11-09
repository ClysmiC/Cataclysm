#pragma once

#include "resource/Resource.h"

struct Mesh;
struct Ecs;

bool loadObjIntoMesh(FilenameString objFile, Mesh* mesh);
bool loadObjSubobjectsAsEntities(FilenameString objFile, Ecs* ecs, bool createMeshes, bool createColliders);


