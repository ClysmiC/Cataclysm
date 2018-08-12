#pragma once

#include "als/als_types.h"

#include "resources/Material.h"
#include "MeshVertex.h"
#include <vector>

#include "Resource.h"

#include "Aabb.h"

struct Mesh;

struct SubmeshOpenGlInfo
{
    uint32 indicesSize;
    uint32 vao;
    uint32 vbo;
    uint32 ebo;
};

// A mesh's data can be broken up into submeshes.
// A submesh is the faces of a mesh that have the same material.
// Thus, each submesh has exactly 1 material.
// It is possible for a mesh to be entirely composed of 1 submesh (if the mesh only has 1 material).
struct Submesh
{
    Submesh() = default;
    Submesh(FilenameString filename, string32 submeshName, const std::vector<MeshVertex> &vertices, const std::vector<uint32> &indices, Material* material, Mesh* mesh);
    
    FilenameString meshFilename;
    string32 submeshName;
    std::vector<MeshVertex> vertices;
    std::vector<uint32> indices;

    Aabb bounds;

    Mesh* mesh;
    Material* material;

    SubmeshOpenGlInfo openGlInfo;
};

void recalculateTangentsAndBitangents(Submesh* submesh);

// Note: only works after MODIFYING vertices... do NOT
// add/remove vertices and then call this
void reuploadModifiedVerticesToGpu(Submesh* submesh);

