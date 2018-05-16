#pragma once

#include <string>
#include "Types.h"

struct Texture
{
	void init(const std::string filename, bool gammaCorrect = false);

    bool load();
    bool unload();

	std::string id;
	bool isLoaded = false;
    bool gammaCorrect;

    static Texture* white();
    static Texture* gray();
    static Texture* black();
    static Texture* defaultNormal();

    uint32 openGlHandle;
};

