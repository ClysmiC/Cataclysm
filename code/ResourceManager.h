#pragma once

#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

#include <unordered_map>

typedef uint32 RESOURCE_HANDLE;

struct ResourceManager
{
    RESOURCE_HANDLE nextHandle;
	
    static ResourceManager& instance();

    std::string toFullPath(const std::string &relativeResourcePath);

    RESOURCE_HANDLE getHandle(std::string id);

    void initDefaults();

    Texture* initTexture(std::string relFilename, bool gammaCorrect, bool loadNow);
    Mesh* initMesh(std::string relFilename, bool useMaterialsRefrencedInObjFile, bool loadNow);
    Material* initMaterial(std::string relFilename, std::string materialName, bool loadNow);
    Shader* initShader(std::string vertFilename, std::string fragFilename, bool loadNow);

    Texture* getTexture(std::string relFilename);
    Mesh* getMesh(std::string relFilename);
    Material* getMaterial(std::string id);
    Material* getMaterial(std::string relFilename, std::string materialName);
    Shader* getShader(std::string id);
    Shader* getShader(std::string vertFilename, std::string fragFilename);

    std::string resourceDirectory;

    std::unordered_map<std::string, RESOURCE_HANDLE> idToHandle;

    // Map per resource type
    // These maps are where all of the resources are allocated
    std::unordered_map<RESOURCE_HANDLE, Texture> textures;
    std::unordered_map<RESOURCE_HANDLE, Mesh> meshes;
    std::unordered_map<RESOURCE_HANDLE, Material> materials;
    std::unordered_map<RESOURCE_HANDLE, Shader> shaders;

private:
    template<class T>
    T* initResource(T resource, std::unordered_map<RESOURCE_HANDLE, T> &table, bool loadNow);

    template<class T>
    T* getResource(std::string id, std::unordered_map<RESOURCE_HANDLE, T> &table);
};

std::string truncateFilenameAfterDirectory(std::string filename);

