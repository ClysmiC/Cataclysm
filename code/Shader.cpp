#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "assert.h"
#include "ResourceManager.h"

#include "GL/glew.h"

const std::string Shader::COMPOSITE_ID_DELIMITER = "|";

void Shader::init(std::string vertFilename_, std::string fragFilename_)
{
	this->id = idFromFilenames(vertFilename_, fragFilename_);
	this->vertFilename = vertFilename_;
	this->fragFilename = fragFilename_;
}

bool Shader::bind() const
{
    assert(isLoaded);

    if (!isLoaded) return false;

    glUseProgram(openGlHandle);
    return true;
}

std::string Shader::idFromFilenames(std::string vertFilename, std::string fragFilename)
{
    return vertFilename + COMPOSITE_ID_DELIMITER + fragFilename;
}

void Shader::setBool(const std::string & name, bool value) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform1i(glGetUniformLocation(openGlHandle, name.c_str()), (int)value);
}

void Shader::setInt(const std::string & name, int value) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform1i(glGetUniformLocation(openGlHandle, name.c_str()), value);
}

void Shader::setFloat(const std::string & name, real32 value) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform1f(glGetUniformLocation(openGlHandle, name.c_str()), value);
}

void Shader::setVec2(const std::string & name, Vec2 value) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform2f(glGetUniformLocation(openGlHandle, name.c_str()), value.x, value.y);
}

void Shader::setVec2(const std::string & name, real32 x, real32 y) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform2f(glGetUniformLocation(openGlHandle, name.c_str()), x, y);
}

void Shader::setVec3(const std::string & name, Vec3 value) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform3f(glGetUniformLocation(openGlHandle, name.c_str()), value.x, value.y, value.z);
}

void Shader::setVec3(const std::string & name, real32 x, real32 y, real32 z) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform3f(glGetUniformLocation(openGlHandle, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string & name, Vec4 value) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform4f(glGetUniformLocation(openGlHandle, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::setVec4(const std::string & name, real32 x, real32 y, real32 z, real32 w) const
{
    assert(isLoaded); if (!isLoaded) return;
    glUniform4f(glGetUniformLocation(openGlHandle, name.c_str()), x, y, z, w);
}

void Shader::setMat4(const std::string & name, Mat4 &value) const
{
	// NOTE: Transpose matrix to make it column order
    glUniformMatrix4fv(glGetUniformLocation(openGlHandle, name.c_str()), 1, GL_TRUE, value.dataPointer());
}

bool Shader::load()
{
    // TODO: return false if file(s) not found
    if (isLoaded) return true;

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vStream(ResourceManager::instance().toFullPath(vertFilename));
    std::ifstream fStream(ResourceManager::instance().toFullPath(fragFilename));

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vStream.rdbuf();
    fShaderStream << fStream.rdbuf();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();

    uint32 vertex, fragment;

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    
    // verify vertex shader
    {
        int success;
        char infoLog[1024];
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
            __debugbreak(); // check infoLog variable

            false;
        }
    }

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    
    // verify fragment shader
    {
        int success;
        char infoLog[1024];
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
            __debugbreak(); // check infoLog variable

            false;
        }
    }

    // shader Program
    openGlHandle = glCreateProgram();

    glAttachShader(openGlHandle, vertex);
    glAttachShader(openGlHandle, fragment);
    glLinkProgram(openGlHandle);

    // verify link
    {
        int success;
        char infoLog[1024];
        glGetProgramiv(openGlHandle, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(openGlHandle, 1024, NULL, infoLog);
            __debugbreak(); // check infoLog variable

            false;
        }
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    isLoaded = true;
    return true;
}

bool Shader::unload()
{
    glDeleteProgram(openGlHandle);
    openGlHandle = 0;
    isLoaded = false;

    return true;
}
