#pragma once

#include <vector>
#include "Texture.h"
#include "resource/MeshVertex.h"
#include "resource/Submesh.h"
#include "Aabb.h"

#include "als/als_math.h"

#include "resource/Resource.h"

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

