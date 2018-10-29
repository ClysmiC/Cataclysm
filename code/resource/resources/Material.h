#pragma once

#include <unordered_map>

#include "als/als_types.h"
#include "resource/Resource.h"
#include "als/als_fixed_string_std_hash.h"

typedef string32 MaterialNameString;
typedef string64 UniformNameString;

//
// Forward declarations
//
struct Texture;
struct Shader;
union Vec2;
union Vec3;
union Vec4;
struct Mat4;

struct Material
{
    static const char COMPOSITE_ID_DELIMITER;
    static const FilenameString DEFAULT_MATERIAL_FILENAME;
    static const MaterialNameString DEFAULT_MATERIAL_NAME;
    static const FilenameString ERROR_MATERIAL_FILENAME;
    static const MaterialNameString ERROR_MATERIAL_NAME;

    Material() = default;
    Material(FilenameString filename, MaterialNameString name);
    
    Shader *shader;
    std::unordered_map<UniformNameString, int32> intUniforms;
    std::unordered_map<UniformNameString, bool> boolUniforms;
    std::unordered_map<UniformNameString, float32> floatUniforms;
    std::unordered_map<UniformNameString, Vec2> vec2Uniforms;
    std::unordered_map<UniformNameString, Vec3> vec3Uniforms;
    std::unordered_map<UniformNameString, Vec4> vec4Uniforms;
    std::unordered_map<UniformNameString, Mat4> mat4Uniforms;
    std::unordered_map<UniformNameString, Texture*> textureUniforms;

    bool isLoaded = false;
    bool receiveLight = true;
    
    ResourceIdString id;
    FilenameString filename;
    MaterialNameString name;
};

bool load(Material* material);
bool loadFromMtlFile(Material* material, FilenameString fullFilename);
bool unload(Material* material);
void clearUniforms(Material* material);
bool bind(Material* material);
void bindShadowMap(Material* material, uint32 shadowMapTextureId);

