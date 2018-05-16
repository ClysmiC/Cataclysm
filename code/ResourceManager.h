#pragma once

#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Cubemap.h"

#include <unordered_map>

typedef uint32 RESOURCE_HANDLE;

struct ResourceManager
{
    RESOURCE_HANDLE nextHandle;
	
    static ResourceManager& instance();
	void init();

    std::string toFullPath(const std::string &relativeResourcePath);

    RESOURCE_HANDLE getHandle(std::string id);


    Texture* initTexture(std::string relFilename, bool gammaCorrect, bool loadNow);
    Texture* getTexture(std::string relFilename);
	
    Mesh* initMesh(std::string relFilename, bool useMaterialsRefrencedInObjFile, bool loadNow);
    Mesh* getMesh(std::string relFilename);
	
    Material* initMaterial(std::string relFilename, std::string materialName, bool loadNow);
    Material* getMaterial(std::string id);
    Material* getMaterial(std::string relFilename, std::string materialName);
	
    Shader* initShader(std::string vertFilename, std::string fragFilename, bool loadNow);
    Shader* getShader(std::string id);
    Shader* getShader(std::string vertFilename, std::string fragFilename);
	
	Cubemap* initCubemap(std::string directoryName, std::string extension, bool loadNow);
	Cubemap* getCubemap(std::string id);

    std::string resourceDirectory;

    std::unordered_map<std::string, RESOURCE_HANDLE> idToHandle;

    // Map per resource type
    // These maps are where all of the resources are allocated
    std::unordered_map<RESOURCE_HANDLE, Texture> textures;
    std::unordered_map<RESOURCE_HANDLE, Mesh> meshes;
    std::unordered_map<RESOURCE_HANDLE, Material> materials;
    std::unordered_map<RESOURCE_HANDLE, Shader> shaders;
	std::unordered_map<RESOURCE_HANDLE, Cubemap> cubemaps;

private:
    void initDefaults();

    template<class T>
    T* initResource(T resource, std::unordered_map<RESOURCE_HANDLE, T> &table, bool loadNow);

    template<class T>
    T* getResource(std::string id, std::unordered_map<RESOURCE_HANDLE, T> &table);
};

std::string truncateFilenameAfterDirectory(std::string filename);

