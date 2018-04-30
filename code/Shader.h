#pragma once

#include <string>
#include "Texture.h"
#include "als_math.h"

struct Shader
{
	void init(std::string vertFile, std::string fragFile);

    bool bind();

	bool isLoaded = false;
	std::string id;
    std::string vertFilename;
    std::string fragFilename;

    uint32 openGlHandle;

	static const std::string COMPOSITE_ID_DELIMITER;

    bool load();
    bool unload();

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, real32 value) const;

    void setVec2(const std::string &name, Vec2 value) const;
    void setVec2(const std::string &name, real32 x, real32 y) const;

    void setVec3(const std::string &name, Vec3 value) const;
    void setVec3(const std::string &name, real32 x, real32 y, real32 z) const;

    void setVec4(const std::string &name, Vec4 value) const;
    void setVec4(const std::string &name, real32 x, real32 y, real32 z, real32 w) const;

    void setMat4(const std::string &name, Mat4 value) const;

    static std::string idFromFilenames(std::string vertFilename, std::string fragFilename);
};

