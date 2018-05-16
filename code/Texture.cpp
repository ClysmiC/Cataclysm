#include "Texture.h"

#include "assert.h"
#include "ResourceManager.h"
#include <unordered_map>
#include "stb/stb_image.h"

#include "GL/glew.h"

void
Texture::init(const std::string filename, bool gammaCorrect_)
{
	this->id = filename;
	this->gammaCorrect = gammaCorrect_;
}

bool
Texture::load()
{
    if (isLoaded) return true;

    // Load file into image
    std::string filename = ResourceManager::instance().toFullPath(id);

    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename.c_str(), &w, &h, &channels, 0);

    GLenum internalFormat; // How to interpret data in the GPU
    GLenum dataFormat;     // How data is stored in-memory
    if (channels == 1)
    {
        internalFormat = GL_RED;
        dataFormat = GL_RED;
    }
    else if (channels == 3)
    {
        internalFormat = gammaCorrect ? GL_SRGB : GL_RGB;
        dataFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        internalFormat = gammaCorrect ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
    }
    else
    {
        assert(false);
        stbi_image_free(data);
        return false;
    }

    // Upload image to opengl texture and store handle
    glGenTextures(1, &openGlHandle);
    glBindTexture(GL_TEXTURE_2D, openGlHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    isLoaded = true;
    return true;
}

bool
Texture::unload()
{
    assert(isLoaded);

    // Unload texture from OpenGL
    glDeleteTextures(1, &openGlHandle);

    isLoaded = false;

    return true;
}

Texture*
Texture::white()
{
    std::string texFile = "default/white.png";
    ResourceManager::instance().initTexture(texFile, true, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}

Texture*
Texture::gray()
{
    std::string texFile = "default/gray.png";
    ResourceManager::instance().initTexture(texFile, true, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}

Texture*
Texture::black()
{
    std::string texFile = "default/black.png";
    ResourceManager::instance().initTexture(texFile, true, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}

Texture*
Texture::defaultNormal()
{
    std::string texFile = "default/normal.png";
    ResourceManager::instance().initTexture(texFile, false, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}
