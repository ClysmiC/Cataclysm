#pragma once

#include "als/als_types.h"
#include "resource/Resource.h"
#include "GL/glew.h"

//
// Texture is a wrapper around TextureData that also acts as a resource for the resource manager.
// TextureData can be used on its own for textures that aren't read from files, such as shadow map textures.
//
struct TextureData
{
    GLenum gpuFormat;
    
    uint32 width;
    uint32 height;
    
    GLuint textureId = 0;
};

void load(TextureData* textureData, unsigned char* buffer = nullptr, GLenum bufferFormat = 0);
void unload(TextureData* textureData);

struct Texture
{
    Texture() = default;
    Texture(FilenameString filename, bool gammaCorrect = false);

    ResourceIdString id;
    bool isLoaded = false;
    bool gammaCorrect;

    static Texture* white();
    static Texture* gray();
    static Texture* black();
    static Texture* defaultNormal();

    TextureData textureData;
};

bool load(Texture* texture);
bool unload(Texture* texture);

