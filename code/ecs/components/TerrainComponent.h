#pragma once

#include "ecs/Component.h"
#include "als/als_math.h"
#include "resource/resources/Mesh.h"
#include "resource/Resource.h"

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
    
    uint32 xIndex;
    uint32 zIndex;
    
    float32 minHeight;
    float32 maxHeight;

    // For stitching... the position of these vertices is "owned" by an adjacent chunk,
    // but we need a copy of it for rendering
    bool hasExtraXVertex = false;
    bool hasExtraZVertex = false;

    TerrainComponent* terrainComponent;
};
    
struct TerrainComponent : public Component
{
    TerrainComponent() = default;
    TerrainComponent(Entity entity, FilenameString heightmap, Vec3 origin, float32 xLength, float32 zLength, float32 minHeight, float32 maxHeight);

    Vec3 origin;
    
    float32 xLengthPerChunk;
    float32 zLengthPerChunk;

    uint32 xChunkCount;
    uint32 zChunkCount;

    uint32 xVerticesPerChunk = 32;
    uint32 zVerticesPerChunk = 32;
    
    std::vector<std::vector<TerrainChunk>> chunks;

    static constexpr bool multipleAllowedPerEntity = true;
};

uint32 xVerticesInChunk(TerrainChunk* chunk);
uint32 zVerticesInChunk(TerrainChunk* chunk);
MeshVertex* getTerrainVertex(TerrainComponent* terrain, uint32 xIndex, uint32 zIndex);
float32 getTerrainHeight(TerrainComponent* terrain, float32 xWorld, float32 zWorld);

