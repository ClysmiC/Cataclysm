#pragma once

#include <string>
#include "Types.h"

struct Texture
{
    Texture() = default;
    Texture(std::string filename, bool gammaCorrect = false);


    std::string id;
    bool isLoaded = false;
    bool gammaCorrect;

    static Texture* white();
    static Texture* gray();
    static Texture* black();
    static Texture* defaultNormal();

    uint32 openGlHandle;
};

bool load(Texture* texture);
bool unload(Texture* texture);

