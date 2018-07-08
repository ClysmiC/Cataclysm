#pragma once

#include "Component.h"
#include "als_math.h"
#include "Mesh.h"
#include "Resource.h"

#include <vector>

struct MeshVertex;
    
// Terrain is defined by grid of chunks, each which has a mesh of a grid of x, z coordinates with variable y values.
// Each chunk is responsible for drawing a stitching mesh that stiches it to adjacent terrain chunks in increasing X and Z.
// The origin of the terrain component is at the smallest x and z value.
//
// origin       x ->
//       *---*---*---*---* [ *---*---*---*---* 
//     z | \ | \ | \ | \ | S | \ | \ | \ | \ |
//     | *---*---*---*---* T *---*---*---*---*
//     v | \ | \ | \ | \ | I | \ | \ | \ | \ |
//       *---*---*---*---* T *---*---*---*---*
//       [   STITCHING   ] C [   STITCHING   ]
//       *---*---*---*---* H *---*---*---*---*
//       | \ | \ | \ | \ | I | \ | \ | \ | \ |
//       *---*---*---*---* N *---*---*---*---*
//       | \ | \ | \ | \ | G | \ | \ | \ | \ |
//       *---*---*---*---* ] *---*---*---*---*

struct TerrainComponent;

struct TerrainChunk
{
    // We own this mesh, NOT the resource manager
    Mesh mesh;

    // Visually stitch the edge of this mesh to adjacent terrain chunks in increasing X and Z
    Mesh stitching; // TODO
    
    uint32 xIndex;
    uint32 zIndex;
    
    float32 minHeight;
    float32 maxHeight;

    TerrainComponent* terrainComponent;
};
    
struct TerrainComponent : public Component
{
    TerrainComponent() = default;
    TerrainComponent(FilenameString heightmap, Vec3 origin, float32 xLength, float32 zLength, float32 minHeight, float32 maxHeight);

    Vec3 origin;
    
    float32 xLengthPerChunk;
    float32 zLengthPerChunk;

    uint32 xChunkCount;
    uint32 zChunkCount;

    uint32 xVerticesPerChunk = 32;
    uint32 zVerticesPerChunk = 32;
    
    std::vector<std::vector<TerrainChunk>> chunks;
};

MeshVertex* getTerrainVertex(TerrainComponent* terrain, uint32 xIndex, uint32 zIndex);
float32 getTerrainHeight(TerrainComponent* terrain, float32 xWorld, float32 zWorld);

