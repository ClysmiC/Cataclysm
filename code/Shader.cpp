#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "assert.h"
#include "ResourceManager.h"

#include "GL/glew.h"

const std::string Shader::COMPOSITE_ID_DELIMITER = "|";

Shader::Shader(std::string vertFilename, std::string fragFilename)
{
    this->id = shaderIdFromFilenames(vertFilename, fragFilename);
    this->vertFilename = vertFilename;
    this->fragFilename = fragFilename;
}

bool bind(Shader* shader)
{
    assert(shader->isLoaded);

    if (!shader->isLoaded) return false;

    glUseProgram(shader->openGlHandle);
    return true;
}

std::string shaderIdFromFilenames(std::string vertFilename, std::string fragFilename)
{
    return vertFilename + Shader::COMPOSITE_ID_DELIMITER + fragFilename;
}

void setBool(Shader* shader, std::string name, bool value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform1i(glGetUniformLocation(shader->openGlHandle, name.c_str()), (int)value);
}

void setInt(Shader* shader, std::string name, int value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform1i(glGetUniformLocation(shader->openGlHandle, name.c_str()), value);
}

void setFloat(Shader* shader, std::string name, real32 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform1f(glGetUniformLocation(shader->openGlHandle, name.c_str()), value);
}

void setVec2(Shader* shader, std::string name, Vec2 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform2f(glGetUniformLocation(shader->openGlHandle, name.c_str()), value.x, value.y);
}

void setVec2(Shader* shader, std::string name, real32 x, real32 y)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform2f(glGetUniformLocation(shader->openGlHandle, name.c_str()), x, y);
}

void setVec3(Shader* shader, std::string name, Vec3 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    auto loc = glGetUniformLocation(shader->openGlHandle, name.c_str());
    glUniform3f(loc, value.x, value.y, value.z);
}

void setVec3(Shader* shader, std::string name, real32 x, real32 y, real32 z)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform3f(glGetUniformLocation(shader->openGlHandle, name.c_str()), x, y, z);
}

void setVec4(Shader* shader, std::string name, Vec4 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform4f(glGetUniformLocation(shader->openGlHandle, name.c_str()), value.x, value.y, value.z, value.w);
}

void setVec4(Shader* shader, std::string name, real32 x, real32 y, real32 z, real32 w)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform4f(glGetUniformLocation(shader->openGlHandle, name.c_str()), x, y, z, w);
}

void setMat4(Shader* shader, std::string name, Mat4 &value)
{
    // NOTE: Transpose matrix to make it column order
    glUniformMatrix4fv(glGetUniformLocation(shader->openGlHandle, name.c_str()), 1, GL_TRUE, value.dataPointer());
}

bool load(Shader* shader)
{
    // TODO: return false if file(s) not found
    if (shader->isLoaded) return true;

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vStream(ResourceManager::instance().toFullPath(shader->vertFilename));
    std::ifstream fStream(ResourceManager::instance().toFullPath(shader->fragFilename));

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vStream.rdbuf();
    fShaderStream << fStream.rdbuf();

    if (vStream.fail() || fStream.fail())
    {
        assert(false);
        return false;
    }

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

            return false;
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

            return false;
        }
    }

    // shader Program
    shader->openGlHandle = glCreateProgram();

    glAttachShader(shader->openGlHandle, vertex);
    glAttachShader(shader->openGlHandle, fragment);
    glLinkProgram(shader->openGlHandle);

    // verify link
    {
        int success;
        char infoLog[1024];
        glGetProgramiv(shader->openGlHandle, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader->openGlHandle, 1024, NULL, infoLog);
            __debugbreak(); // check infoLog variable

            return false;
        }
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    shader->isLoaded = true;
    return true;
}

bool unload(Shader* shader)
{
    glDeleteProgram(shader->openGlHandle);
    shader->openGlHandle = 0;
    shader->isLoaded = false;

    return true;
}
