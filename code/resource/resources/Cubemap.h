#pragma once

#include "resource/Resource.h"
#include "als/als_types.h"

struct ITransform;
struct CameraComponent;
struct Shader;

struct Cubemap
{
    Cubemap() = default;
    Cubemap(ResourceIdString directoryName, string8 extension_);

    ResourceIdString id;
    string8 extension;
    uint32 openGlHandle;
    bool isLoaded = false;
};

Shader* cubemapShader();

bool load(Cubemap* cubemap);
bool unload(Cubemap* cubemap);
void renderCubemap(Cubemap* cubemap, CameraComponent* camera, ITransform* cameraXfm);

uint32 cubemapVao();
