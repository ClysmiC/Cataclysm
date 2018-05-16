#pragma once

#include <string>
#include "Types.h"

struct Cubemap
{
	void init(const std::string& directory, const std::string& extension);

	bool load();
	bool unload();

	std::string id;
	std::string extension;
	
	bool isLoaded = false;

	uint32 openGlHandle;
};
