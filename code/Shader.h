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

bool load();
bool unload();

void setBool(std::string &name, bool value);
void setInt(std::string &name, int value);
void setFloat(std::string &name, real32 value);

void setVec2(std::string &name, Vec2 value);
void setVec2(std::string &name, real32 x, real32 y);

void setVec3(std::string &name, Vec3 value);
void setVec3(std::string &name, real32 x, real32 y, real32 z);

void setVec4(std::string &name, Vec4 value);
void setVec4(std::string &name, real32 x, real32 y, real32 z, real32 w);

void setMat4(std::string &name, Mat4 &value);

bool bind(Shader* shader);

std::string shaderIdFromFilenames(std::string vertFilename, std::string fragFilename);

