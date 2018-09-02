#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include "assert.h"

#include "resources/Material.h"
#include "resources/Shader.h"
#include "resources/Texture.h"
#include "resources/Mesh.h"
#include "resources/Cubemap.h"

ResourceManager & ResourceManager::instance()
{
    static ResourceManager *instance = new ResourceManager();

    return *instance;
}

void ResourceManager::init()
{
    resourceDirectory = "x:/resources/";
    initDefaults();
}

FilenameString ResourceManager::toFullPath(FilenameString relativeResourcePath)
{
    return resourceDirectory + relativeResourcePath;
}

RESOURCE_HANDLE ResourceManager::getHandle(ResourceIdString id)
{
    auto handle = idToHandle.find(id);
    if (handle == idToHandle.end())
    {
        return 0;
    }

    return handle->second;
}

void ResourceManager::initDefaults()
{
    initMaterial(Material::DEFAULT_MATERIAL_FILENAME, Material::DEFAULT_MATERIAL_NAME, true);
    initMaterial(Material::ERROR_MATERIAL_FILENAME, Material::ERROR_MATERIAL_NAME, true);
}

Texture* ResourceManager::getTexture(ResourceIdString id)
{
    return getResource(id, textures);
}

Mesh* ResourceManager::getMesh(ResourceIdString id)
{
    return getResource(id, meshes);
}

Material* ResourceManager::getMaterial(ResourceIdString id)
{
    return getResource(id, materials);
}

Material* ResourceManager::getMaterial(FilenameString relFilename, MaterialNameString materialName)
{
    ResourceIdString id = relFilename + Material::COMPOSITE_ID_DELIMITER + materialName;
    return getResource(id, materials);
}

Cubemap*
ResourceManager::getCubemap(ResourceIdString id)
{
    return getResource(id, cubemaps);
}

Shader* ResourceManager::getShader(ResourceIdString id)
{
    return getResource(id, shaders);
}

Shader* ResourceManager::getShader(FilenameString vertFilename, FilenameString fragFilename)
{
    return getShader(shaderIdFromFilenames(vertFilename, fragFilename));
}

Texture* ResourceManager::initTexture(FilenameString relFilename, bool gammaCorrect, bool loadNow)
{
    Texture t(relFilename, gammaCorrect);
    return initResource(t, textures, loadNow);
}

Material* ResourceManager::initMaterial(FilenameString relFilename, MaterialNameString materialName, bool loadNow)
{
    Material material(relFilename, materialName);
    return initResource(material, materials, loadNow);
}

Shader* ResourceManager::initShader(FilenameString vertFilename, FilenameString fragFilename, bool loadNow)
{
    Shader shader(vertFilename, fragFilename);
    return initResource(shader, shaders, loadNow);
}

Mesh* ResourceManager::initMesh(FilenameString relFilename, bool useMaterialsRefrencedInObjFile, MeshLoadOptions loadNow)
{
    Mesh m(relFilename, useMaterialsRefrencedInObjFile);
    Mesh* result = initResource(m, meshes, loadNow == MeshLoadOptions::CPU || loadNow == MeshLoadOptions::CPU_AND_GPU);

    if (loadNow == MeshLoadOptions::CPU_AND_GPU)
    {
        uploadToGpuOpenGl(result);
    }

    return result;
}

Cubemap*
ResourceManager::initCubemap(FilenameString directoryName, FileExtensionString extension, bool loadNow)
{
    Cubemap c(directoryName, extension);
    return initResource(c, cubemaps, loadNow);
}

template<class T>
T* ResourceManager::initResource(T resourceArg, std::unordered_map<RESOURCE_HANDLE, T> &table, bool loadNow)
{
    auto it = idToHandle.find(resourceArg.id);
    if (it != idToHandle.end())
    {
        if (loadNow)
        {
            T &r = table.at(it->second);
            load(&r);
            return &r;
        }

        return &(table.at(it->second)); // already inited;
    }

    RESOURCE_HANDLE handle = nextHandle++;

    idToHandle[resourceArg.id] = handle;

    table.emplace(handle, resourceArg);

    T *allocatedResource = &table.at(handle);

    if (loadNow)
    {
        load(allocatedResource);
        return allocatedResource;
    }

    return allocatedResource;
}

template<class T>
T* ResourceManager::getResource(ResourceIdString id, std::unordered_map<RESOURCE_HANDLE, T> &table)
{
    auto it = idToHandle.find(id);
    if (it == idToHandle.end())
    {
        return nullptr;
    }

    RESOURCE_HANDLE handle = idToHandle.at(id);
    T& resource = table.at(handle);

    return &resource;
}

FilenameString truncateFilenameAfterDirectory(FilenameString filename)
{
    int32 index = filename.lastIndexOf('/');

    if (index == -1) return "";

    return filename.substring(0, index + 1);
}
