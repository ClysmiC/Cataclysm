#include "Texture.h"

#include "assert.h"
#include "ResourceManager.h"
#include <unordered_map>
#include "stb/stb_image.h"
#include "als_fixed_string_std_hash.h"

#include "GL/glew.h"

Texture::Texture(FilenameString filename, bool gammaCorrect_)
{
    this->id = filename;
    this->gammaCorrect = gammaCorrect_;
}

bool load(Texture* texture)
{
    if (texture->isLoaded) return true;

    // Load file into image
    FilenameString filename = ResourceManager::instance().toFullPath(texture->id);

    int w, h, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename.cstr(), &w, &h, &channels, 0);

    GLenum internalFormat; // How to interpret data in the GPU
    GLenum dataFormat;     // How data is stored in-memory
    if (channels == 1)
    {
        internalFormat = GL_RED;
        dataFormat = GL_RED;
    }
    else if (channels == 3)
    {
        internalFormat = texture->gammaCorrect ? GL_SRGB : GL_RGB;
        dataFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        internalFormat = texture->gammaCorrect ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
    }
    else
    {
        assert(false);
        stbi_image_free(data);
        return false;
    }

    // Upload image to opengl texture and store handle
    glGenTextures(1, &texture->openGlHandle);
    glBindTexture(GL_TEXTURE_2D, texture->openGlHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    texture->isLoaded = true;
    return true;
}

bool unload(Texture* texture)
{
    assert(texture->isLoaded);

    // Unload texture from OpenGL
    glDeleteTextures(1, &texture->openGlHandle);

    texture->isLoaded = false;

    return true;
}

Texture*
Texture::white()
{
    FilenameString texFile = "default/white.png";
    ResourceManager::instance().initTexture(texFile, true, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}

Texture*
Texture::gray()
{
    FilenameString texFile = "default/gray.png";
    ResourceManager::instance().initTexture(texFile, true, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}

Texture*
Texture::black()
{
    FilenameString texFile = "default/black.png";
    ResourceManager::instance().initTexture(texFile, true, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}

Texture*
Texture::defaultNormal()
{
    FilenameString texFile = "default/normal.png";
    ResourceManager::instance().initTexture(texFile, false, true);
    Texture* tex = ResourceManager::instance().getTexture(texFile);
    assert(tex != nullptr);
    return tex;
}
