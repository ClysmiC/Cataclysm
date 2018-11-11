#include "Mesh.h"
#include <fstream>
#include <sstream>
#include "assert.h"
#include <vector>
#include <unordered_map>
#include <tuple>
#include <unordered_set>
#include "resource/ResourceManager.h"
#include <algorithm>
#include "ObjLoading.h"

Mesh::Mesh(FilenameString filename, bool useMaterialsReferencedInObjFile)
{
    this->id = filename;
    this->filename = filename;
    this->useMaterialsReferencedInObjFile = useMaterialsReferencedInObjFile;
}

Mesh::Mesh(FilenameString filename, string64 subObjectName)
{
    this->id = filename + "|" + subObjectName;
    this->filename = filename;
    this->subObjectName = subObjectName;
    this->useMaterialsReferencedInObjFile = false;
}

uint32 meshVerticesCount(Mesh* mesh)
{
    if (!mesh) return 0;

    uint32 count = 0;
    for (uint32 i = 0; i < mesh->submeshes.size(); i++)
    {
        count += mesh->submeshes[i].vertices.size();
    }

    return count;
}

bool load(Mesh* mesh)
{
    using namespace std;

    if (mesh->isLoaded) return true;

    loadObjIntoMesh(mesh->filename, mesh);

    // @Think: should calculating bounds be done here or in the obj loading function
    //
    // Calculate bounds
    //
    {
        float32 minX = FLT_MAX;
        float32 minY = FLT_MAX;
        float32 minZ = FLT_MAX;

        float32 maxX = -FLT_MAX;
        float32 maxY = -FLT_MAX;
        float32 maxZ = -FLT_MAX;

        for (Submesh& submesh : mesh->submeshes)
        {
            Vec3 minPoint = submesh.bounds.center - submesh.bounds.halfDim;
            Vec3 maxPoint = submesh.bounds.center + submesh.bounds.halfDim;
            
            minX = std::min(minX, minPoint.x);
            minY = std::min(minY, minPoint.y);
            minZ = std::min(minZ, minPoint.z);
        
            maxX = std::max(maxX, maxPoint.x);
            maxY = std::max(maxY, maxPoint.y);
            maxZ = std::max(maxZ, maxPoint.z);
        }

        Vec3 minPoint = Vec3(minX, minY, minZ);
        Vec3 maxPoint = Vec3(maxX, maxY, maxZ);
        
        mesh->bounds.halfDim = Vec3(
            (maxPoint.x - minPoint.x) / 2.0f,
            (maxPoint.y - minPoint.y) / 2.0f,
            (maxPoint.z - minPoint.z) / 2.0f
        );
    
        mesh->bounds.center = minPoint + mesh->bounds.halfDim;
    }

    mesh->isLoaded = true;

    return true;
}

bool unload(Mesh* mesh)
{
    // TODO: what to do... remove self from resource manager?
    return false;
}

void recalculatePositionsRelativeToCentroid(Mesh * mesh, Vec3 centroid)
{
    for (Submesh& sub : mesh->submeshes)
    {
        recalculatePositionsRelativeToCentroid(&sub, centroid);
    }
}

bool isUploadedToGpuOpenGl(Mesh* mesh)
{
    if (mesh) return false;
    if (mesh->submeshes.size() == 0) return false;

    return isUploadedToGpuOpenGl(&mesh->submeshes[0]);
}

void uploadToGpuOpenGl(Mesh* mesh)
{
    // Load all materials
    for (Material* material : mesh->materialsReferencedInObjFile)
    {
        load(material);
    }

    for (uint32 i = 0; i < mesh->submeshes.size(); i++)
    {
        uploadToGpuOpenGl(&(mesh->submeshes[i]));
    }

    //if (mesh->useMaterialsReferencedInObjFile)
    //{

    //}
}
