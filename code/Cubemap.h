#pragma once

#include "Resource.h"
#include "Types.h"

#include "CameraComponent.h"

struct Transform;
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
void renderCubemap(Cubemap* cubemap, CameraComponent* camera, Transform* cameraXfm);

uint32 cubemapVao();
