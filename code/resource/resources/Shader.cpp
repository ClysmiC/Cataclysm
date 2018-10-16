#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "assert.h"
#include "resource/ResourceManager.h"

// std string used for shader bodies. after upload, they get reclaimed,
// so it isn't a huge deal. TODO: look into custom resizeable string for shaders
// or maybe just a gigantic fixed string... it's a one-time cost, nbd
#include <string>

#include "GL/glew.h"

const char Shader::COMPOSITE_ID_DELIMITER = '|';
const FilenameString Shader::SIMPLE_DEPTH_VERT_SHADER = "shader/simpleDepth.vert";
const FilenameString Shader::SIMPLE_DEPTH_FRAG_SHADER = "shader/simpleDepth.frag";

Shader::Shader(string128 vertFilename, string128 fragFilename)
{
    assert(!vertFilename.isTruncated && !fragFilename.isTruncated);
    
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

string256 shaderIdFromFilenames(string128 vertFilename, string128 fragFilename)
{
    string256 result = vertFilename;
    result += Shader::COMPOSITE_ID_DELIMITER;
    result += fragFilename;

    assert(!result.isTruncated);
    
    return result;
}

void setBool(Shader* shader, UniformNameString name, bool value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform1i(glGetUniformLocation(shader->openGlHandle, name.cstr()), (int)value);
}

void setInt(Shader* shader, UniformNameString name, int value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform1i(glGetUniformLocation(shader->openGlHandle, name.cstr()), value);
}

void setFloat(Shader* shader, UniformNameString name, float32 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform1f(glGetUniformLocation(shader->openGlHandle, name.cstr()), value);
}

void setVec2(Shader* shader, UniformNameString name, Vec2 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform2f(glGetUniformLocation(shader->openGlHandle, name.cstr()), value.x, value.y);
}

void setVec2(Shader* shader, UniformNameString name, float32 x, float32 y)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform2f(glGetUniformLocation(shader->openGlHandle, name.cstr()), x, y);
}

void setVec3(Shader* shader, UniformNameString name, Vec3 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    auto loc = glGetUniformLocation(shader->openGlHandle, name.cstr());
    glUniform3f(loc, value.x, value.y, value.z);
}

void setVec3(Shader* shader, UniformNameString name, float32 x, float32 y, float32 z)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform3f(glGetUniformLocation(shader->openGlHandle, name.cstr()), x, y, z);
}

void setVec4(Shader* shader, UniformNameString name, Vec4 value)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform4f(glGetUniformLocation(shader->openGlHandle, name.cstr()), value.x, value.y, value.z, value.w);
}

void setVec4(Shader* shader, UniformNameString name, float32 x, float32 y, float32 z, float32 w)
{
    assert(shader->isLoaded); if (!shader->isLoaded) return;
    glUniform4f(glGetUniformLocation(shader->openGlHandle, name.cstr()), x, y, z, w);
}

void setMat4(Shader* shader, UniformNameString name, Mat4 &value)
{
    // NOTE: Transpose matrix to make it column order
    GLint location = glGetUniformLocation(shader->openGlHandle, name.cstr());
    glUniformMatrix4fv(location, 1, GL_TRUE, value.dataPointer());
}

bool load(Shader* shader)
{
    // TODO: return false if file(s) not found
    if (shader->isLoaded) return true;

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vStream(ResourceManager::instance().toFullPath(shader->vertFilename).cstr());
    std::ifstream fStream(ResourceManager::instance().toFullPath(shader->fragFilename).cstr());

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
