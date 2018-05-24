#pragma once

#include <string>
#include "Types.h"
#include "Shader.h"

#include "CameraComponent.h"

struct GlobalCubemapInfo
{
	static bool isInited;
	static void init();
	
	static Shader* shader;
	static uint32 vao;
	static uint32 vbo;
	static real32 vertices[];
};

struct Cubemap
{
	void init(const std::string& directory, const std::string& extension);

	bool load();
	bool unload();

	std::string id;
	std::string extension;
	
	bool isLoaded = false;

	uint32 openGlHandle;

	void render(CameraComponent* cameraComponent);
};
