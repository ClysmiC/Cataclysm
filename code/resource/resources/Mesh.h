#pragma once

#include <vector>
#include "resource/Submesh.h"
#include "Aabb.h"

#include "resource/Resource.h"

struct Mesh
{
    Mesh() = default;
    Mesh(FilenameString filename, bool useMaterialsReferencedInObjFile);
    
    bool useMaterialsReferencedInObjFile;
    bool isLoaded = false;
    ResourceIdString id;
    std::vector<Submesh> submeshes;
    std::vector<Material*> materialsReferencedInObjFile;

    Aabb bounds;
};

uint32 meshVerticesCount(Mesh* mesh);
bool load(Mesh* mesh);
bool unload(Mesh* mesh);

bool isUploadedToGpuOpenGl(Mesh* mesh);
void uploadToGpuOpenGl(Mesh* mesh);

