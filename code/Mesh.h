#pragma once

#include <vector>
#include "Texture.h"
#include "MeshVertex.h"
#include "Submesh.h"
#include "CameraComponent.h"
#include "Aabb.h"

#include "als_math.h"

#include "Resource.h"

struct Mesh
{
    Mesh() = default;
    Mesh(FilenameString filename, bool useMaterialsReferencedInObjFile);
    
    bool useMaterialsReferencedInObjFile;
    bool isLoaded = false;
    ResourceIdString id;
    std::vector<Submesh> submeshes;

    Aabb bounds;
};

bool load(Mesh* mesh);
bool unload(Mesh* mesh);

