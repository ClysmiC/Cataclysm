#include "Texture.h"

#include "assert.h"
#include "resource/ResourceManager.h"
#include <unordered_map>
#include "stb/stb_image.h"
#include "als/als_fixed_string_std_hash.h"

#include "GL/glew.h"

void load(TextureData* textureData, unsigned char* buffer, GLenum bufferFormat)
{
    assert(textureData->textureId == 0);
    
    // Upload image to opengl texture and store handle
    glGenTextures(1, &textureData->textureId);
    glBindTexture(GL_TEXTURE_2D, textureData->textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    if (buffer)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, textureData->gpuFormat, textureData->width, textureData->height, 0, bufferFormat, GL_UNSIGNED_BYTE, buffer);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, textureData->gpuFormat, textureData->width, textureData->height);
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);

	auto v = glGetError();
	assert(v == GL_NO_ERROR);
}

void unload(TextureData* textureData)
{
    assert(textureData->textureId != 0);

    // Unload texture from OpenGL
    glDeleteTextures(1, &textureData->textureId);

    textureData->textureId = 0;
}

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
    unsigned char *buffer = stbi_load(filename.cstr(), &w, &h, &channels, 0);
    GLenum bufferFormat;

    texture->textureData.width = w;
    texture->textureData.height = h;
    
    if (channels == 1)
    {
        texture->textureData.gpuFormat = GL_RED;
        bufferFormat = GL_RED;
    }
    else if (channels == 3)
    {
        texture->textureData.gpuFormat = texture->gammaCorrect ? GL_SRGB : GL_RGB;
        bufferFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        texture->textureData.gpuFormat = texture->gammaCorrect ? GL_SRGB_ALPHA : GL_RGBA;
        bufferFormat = GL_RGBA;
    }
    else
    {
        assert(false);
        stbi_image_free(buffer);
        return false;
    }

    // Upload image to opengl texture and store handle
    load(&texture->textureData, buffer, bufferFormat);

    stbi_image_free(buffer);
    texture->isLoaded = true;
    return true;
}

bool unload(Texture* texture)
{
    assert(texture->isLoaded);

    unload(&texture->textureData);

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
