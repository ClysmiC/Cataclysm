#pragma once

#include "Component.h"
#include "als_math.h"
#include "Mesh.h"
#include "Resource.h"

struct MeshVertex;
    
// Terrain is defined by a mesh of a grid of x, z coordinates with variable y values.
// The origin of the mesh is at the smallest x and z value.
// The mesh looks something like this
//
// origin       x ->
//       *---*---*---*---*
//     z | \ | \ | \ | \ |
//     | *---*---*---*---*
//     v | \ | \ | \ | \ |
//       *---*---*---*---*

struct TerrainComponent : public Component
{
    TerrainComponent() = default;
    TerrainComponent(FilenameString heightmap, Vec3 origin, float32 xLength, float32 zLength, float32 minHeight, float32 maxHeight);
    
    // We own this mesh, NOT the resource manager
    Mesh mesh;
    
    Vec3 origin;
    
    float32 minHeight;
    float32 maxHeight;
    
    float32 xLength;
    float32 zLength;
                     
    uint32 xVertexCount;
    uint32 zVertexCount;
};

MeshVertex* getTerrainVertex(TerrainComponent* terrain, uint32 xIndex, uint32 zIndex);
float32 getTerrainHeight(TerrainComponent* terrain, float32 xWorld, float32 zWorld);

