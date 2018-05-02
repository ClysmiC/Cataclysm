#pragma once

#include "Types.h"

#include "Material.h"
#include "MeshVertex.h"
#include <vector>
#include "Camera.h"

struct SubmeshOpenGlInfo
{
	uint32 indicesSize;
	uint32 VAO;
    uint32 VBO;
    uint32 EBO;
};

// A mesh's data can be broken up into submeshes.
// A submesh is the faces of a mesh that have the same material.
// Thus, each submesh has exactly 1 material.
// It is possible for a mesh to be entirely composed of 1 submesh (if the mesh only has 1 material).
struct Submesh
{
	Submesh(const std::string filename, const std::string submeshName, const std::vector<MeshVertex> &vertices, const std::vector<uint32> &indices, Material* material);

	
    std::string meshFilename;
    std::string submeshName;
    std::vector<MeshVertex> vertices;
    std::vector<uint32> indices;

    void draw(const Mat4 &transform, Camera camera);
    void recalculateTangentsAndBitangents();

    Material* material;

	SubmeshOpenGlInfo openGlHandles;

private:
    void setupGl();
};

