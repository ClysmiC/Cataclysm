#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include "assert.h"

ResourceManager & ResourceManager::instance()
{
    static ResourceManager *instance = new ResourceManager();

    return *instance;
}

void ResourceManager::init()
{
    resourceDirectory = "x:/code/resources/";
    initDefaults();
}

std::string ResourceManager::toFullPath(const std::string &relativeResourcePath)
{
    return resourceDirectory + relativeResourcePath;
}

RESOURCE_HANDLE ResourceManager::getHandle(std::string id)
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
}

Texture* ResourceManager::getTexture(std::string id)
{
    return getResource(id, textures);
}

Mesh* ResourceManager::getMesh(std::string id)
{
    return getResource(id, meshes);
}

Material* ResourceManager::getMaterial(std::string id)
{
    return getResource(id, materials);
}

Material* ResourceManager::getMaterial(std::string relFilename, std::string materialName)
{
    return getResource(relFilename + Material::COMPOSITE_ID_DELIMITER + materialName, materials);
}

Shader* ResourceManager::getShader(std::string id)
{
    return getResource(id, shaders);
}

Shader* ResourceManager::getShader(std::string vertFilename, std::string fragFilename)
{
    return getShader(shaderIdFromFilenames(vertFilename, fragFilename));
}

Texture* ResourceManager::initTexture(std::string relFilename, bool gammaCorrect, bool loadNow)
{
    Texture t(relFilename, gammaCorrect);
    return initResource(t, textures, loadNow);
}

Material* ResourceManager::initMaterial(std::string relFilename, std::string materialName, bool loadNow)
{
    Material material(relFilename, materialName);
    return initResource(material, materials, loadNow);
}

Shader* ResourceManager::initShader(std::string vertFilename, std::string fragFilename, bool loadNow)
{
    Shader shader(vertFilename, fragFilename);
    return initResource(shader, shaders, loadNow);
}

Mesh* ResourceManager::initMesh(std::string relFilename, bool useMaterialsRefrencedInObjFile, bool loadNow)
{
    Mesh m(relFilename, useMaterialsRefrencedInObjFile);
    return initResource(m, meshes, loadNow);
}

Cubemap*
ResourceManager::initCubemap(std::string directoryName, std::string extension, bool loadNow)
{
	Cubemap c(directoryName, extension);
	return initResource(c, cubemaps, loadNow);
}

Cubemap*
ResourceManager::getCubemap(std::string id)
{
	return getResource(id, cubemaps);
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
T* ResourceManager::getResource(std::string id, std::unordered_map<RESOURCE_HANDLE, T> &table)
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

std::string truncateFilenameAfterDirectory(std::string filename)
{
	size_t index = filename.find_last_of('/');

	if (index == std::string::npos) return "";

	return filename.substr(0, index + 1);
}
