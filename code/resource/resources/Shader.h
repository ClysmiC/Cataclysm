#pragma once

#include "Texture.h"
#include "als/als_math.h"
#include "als/als_fixed_string.h"
#include "Material.h"

struct Shader
{
    Shader() = default;
    Shader(string128 vertFile, string128 fragFile);

    bool isLoaded = false;

    string256 id;
    string128 vertFilename;
    string128 fragFilename;

    uint32 openGlHandle;

    static const char COMPOSITE_ID_DELIMITER;
};

bool load(Shader* shader);
bool unload(Shader* shader);

void setBool (Shader* shader, UniformNameString name, bool value);
void setInt  (Shader* shader, UniformNameString name, int value);
void setFloat(Shader* shader, UniformNameString name, float32 value);

void setVec2(Shader* shader, UniformNameString name, Vec2 value);
void setVec2(Shader* shader, UniformNameString name, float32 x, float32 y);

void setVec3(Shader* shader, UniformNameString name, Vec3 value);
void setVec3(Shader* shader, UniformNameString name, float32 x, float32 y, float32 z);

void setVec4(Shader* shader, UniformNameString name, Vec4 value);
void setVec4(Shader* shader, UniformNameString name, float32 x, float32 y, float32 z, float32 w);

void setMat4(Shader* shader, UniformNameString name, Mat4 &value);

bool bind(Shader* shader);

string256 shaderIdFromFilenames(string128 vertFilename, string128 fragFilename);

