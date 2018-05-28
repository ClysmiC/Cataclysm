#pragma once

#include <string>
#include "Types.h"
#include "Shader.h"

#include "CameraComponent.h"

struct Cubemap
{
	static real32 vertices[];
	static uint32 vao();
	static uint32 vbo();
	static Shader* shader();
	
	void init(const std::string& directory, const std::string& extension);

	bool load();
	bool unload();

	std::string id;
	std::string extension;
	
	bool isLoaded = false;

	uint32 openGlHandle;

	void render(CameraEntity camera);

private:
	static void initVboAndVao();
	static uint32 vbo_;
	static uint32 vao_;
};
