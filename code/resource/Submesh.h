#pragma once

#include "als/als_types.h"

#include "MeshVertex.h"
#include <vector>

#include "Resource.h"

#include "Aabb.h"

struct Mesh;
struct Material;

struct SubmeshOpenGlInfo
{
    uint32 indicesSize = 0;
    uint32 vao = 0;
    uint32 vbo = 0;
    uint32 ebo = 0;
};

// A mesh's data can be broken up into submeshes.
// A submesh is the faces of a mesh that have the same material.
// Thus, each submesh has exactly 1 material.
// It is possible for a mesh to be entirely composed of 1 submesh (if the mesh only has 1 material).
struct Submesh
{
    Submesh() = default;
    Submesh(FilenameString filename, string64 submeshName, const std::vector<MeshVertex> &vertices, const std::vector<uint32> &indices, Material* material, Mesh* mesh, bool uploadToGpu=true);
    
    FilenameString meshFilename;
    string64 submeshName;
    std::vector<MeshVertex> vertices;
    std::vector<uint32> indices;

    Aabb bounds;

    Mesh* mesh;
    Material* material;

    SubmeshOpenGlInfo openGlInfo;
};

void recalculateTangentsAndBitangents(Submesh* submesh);

bool isUploadedToGpuOpenGl(Submesh* submesh);
void uploadToGpuOpenGl(Submesh* submesh);

void recalculatePositionsRelativeToCentroid(Submesh* submesh, Vec3 centroid);

// Note: only works after MODIFYING vertices... do NOT
// add/remove vertices and then call this
void reuploadModifiedVerticesToGpu(Submesh* submesh);

