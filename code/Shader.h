#pragma once

#include <string>
#include "Texture.h"
#include "als_math.h"

struct Shader
{
    Shader() = default;
    Shader(std::string vertFile, std::string fragFile);

    bool isLoaded = false;
    std::string id;
    std::string vertFilename;
    std::string fragFilename;

    uint32 openGlHandle;

    static const std::string COMPOSITE_ID_DELIMITER;
};

bool load(Shader* shader);
bool unload(Shader* shader);

void setBool(Shader* shader, std::string name, bool value);
void setInt(Shader* shader, std::string name, int value);
void setFloat(Shader* shader, std::string name, float32 value);

void setVec2(Shader* shader, std::string name, Vec2 value);
void setVec2(Shader* shader, std::string name, float32 x, float32 y);

void setVec3(Shader* shader, std::string name, Vec3 value);
void setVec3(Shader* shader, std::string name, float32 x, float32 y, float32 z);

void setVec4(Shader* shader, std::string name, Vec4 value);
void setVec4(Shader* shader, std::string name, float32 x, float32 y, float32 z, float32 w);

void setMat4(Shader* shader, std::string name, Mat4 &value);

bool bind(Shader* shader);

std::string shaderIdFromFilenames(std::string vertFilename, std::string fragFilename);

