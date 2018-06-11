#pragma once

#include <string>
#include "Types.h"
#include "Shader.h"

#include "CameraComponent.h"

struct Cubemap
{
	Cubemap() = default;
	Cubemap(std::string directoryName, std::string extension_);

	std::string id;
	std::string extension;
	uint32 openGlHandle;
	bool isLoaded = false;
};

Shader* cubemapShader();

bool load(Cubemap* cubemap);
bool unload(Cubemap* cubemap);
void renderCubemap(Cubemap* cubemap, CameraComponent* camera, Transform* cameraXfm);

uint32 cubemapVao();
