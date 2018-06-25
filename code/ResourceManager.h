#pragma once

#include "Resource.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Cubemap.h"

#include "als_fixed_string_std_hash.h"

#include <unordered_map>

typedef uint32 RESOURCE_HANDLE;

struct ResourceManager
{
    RESOURCE_HANDLE nextHandle;
    
    //
    // Members
    //
    FilenameString resourceDirectory;

    std::unordered_map<ResourceIdString, RESOURCE_HANDLE> idToHandle;

    // Map per resource type
    // These maps are where all of the resources are allocated
    std::unordered_map<RESOURCE_HANDLE, Texture> textures;
    std::unordered_map<RESOURCE_HANDLE, Mesh> meshes;
    std::unordered_map<RESOURCE_HANDLE, Material> materials;
    std::unordered_map<RESOURCE_HANDLE, Shader> shaders;
    std::unordered_map<RESOURCE_HANDLE, Cubemap> cubemaps;


    //
    // Methods
    //
    void init();
    static ResourceManager& instance();

    FilenameString toFullPath(FilenameString relativeResourcePath);
    RESOURCE_HANDLE getHandle(ResourceIdString id);

    Texture* initTexture(FilenameString relFilename, bool gammaCorrect, bool loadNow);
    Texture* getTexture(ResourceIdString relFilename);
    
    Mesh* initMesh(FilenameString relFilename, bool useMaterialsRefrencedInObjFile, bool loadNow);
    Mesh* getMesh(ResourceIdString relFilename);
    
    Material* initMaterial(FilenameString relFilename, MaterialNameString materialName, bool loadNow);
    Material* getMaterial(ResourceIdString id);
    Material* getMaterial(FilenameString relFilename, MaterialNameString materialName);
    
    Shader* initShader(FilenameString vertFilename, FilenameString fragFilename, bool loadNow);
    Shader* getShader(ResourceIdString id);
    Shader* getShader(FilenameString vertFilename, FilenameString fragFilename);
    
    Cubemap* initCubemap(FilenameString directoryName, FileExtensionString extension, bool loadNow);
    Cubemap* getCubemap(ResourceIdString id);


private:
    void initDefaults();

    template<class T>
    T* initResource(T resource, std::unordered_map<RESOURCE_HANDLE, T> &table, bool loadNow);

    template<class T>
    T* getResource(ResourceIdString id, std::unordered_map<RESOURCE_HANDLE, T> &table);
};

FilenameString truncateFilenameAfterDirectory(FilenameString filename);

