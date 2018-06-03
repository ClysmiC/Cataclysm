#pragma once

#include "Shader.h"
#include "Texture.h"

#include <unordered_map>
#include <string>

struct Material
{
	static const std::string COMPOSITE_ID_DELIMITER;
    static const std::string DEFAULT_MATERIAL_FILENAME;
    static const std::string DEFAULT_MATERIAL_NAME;
	static const std::string DEBUG_MATERIAL_FILENAME;
    static const std::string DEBUG_MATERIAL_NAME;

	Material() = default;
	Material(std::string filename, std::string name);
	
    Shader *shader;
    std::unordered_map<std::string, int32> intUniforms;
    std::unordered_map<std::string, bool> boolUniforms;
    std::unordered_map<std::string, real32> floatUniforms;
    std::unordered_map<std::string, Vec2> vec2Uniforms;
    std::unordered_map<std::string, Vec3> vec3Uniforms;
    std::unordered_map<std::string, Vec4> vec4Uniforms;
    std::unordered_map<std::string, Mat4> mat4Uniforms;
    std::unordered_map<std::string, Texture*> textureUniforms;

	bool isLoaded = false;
	bool receiveLight = true;
	
	std::string id;
    std::string filename;
    std::string name;
};

bool load(Material* material);
bool loadFromMtlFile(Material* material, std::string fullFilename);
bool unload(Material* material);
void clearUniforms(Material* material);
bool bind(Material* material);

