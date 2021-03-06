#pragma once

#include <vector>
#include <unordered_map>
#include "resource/Submesh.h"
#include "Aabb.h"

#include "resource/Resource.h"

struct Mesh
{
    Mesh() = default;
    Mesh(FilenameString filename, bool useMaterialsReferencedInObjFile); // entire file
    Mesh(FilenameString filename, string64 subObjectName); // from a specific point in the obj file until it reaches the next "o" or eof
    
    bool useMaterialsReferencedInObjFile = false;
    bool isLoaded = false;
    ResourceIdString id;
    FilenameString filename;
    string64 subObjectName = "";
    std::vector<Submesh> submeshes;
    std::unordered_map<std::string, Material*> materialsReferencedInObjFile;

    Aabb bounds;
};

uint32 meshVerticesCount(Mesh* mesh);
bool load(Mesh* mesh);
bool unload(Mesh* mesh);

void recalculatePositionsRelativeToCentroid(Mesh* mesh, Vec3 centroid);
void recalculateBounds(Mesh* mesh);

bool isUploadedToGpuOpenGl(Mesh* mesh);
void uploadToGpuOpenGl(Mesh* mesh);

