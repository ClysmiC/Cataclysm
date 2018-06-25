#pragma once

#include <string>
#include "Types.h"
#include "Resource.h"

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

    uint32 openGlHandle;
};

bool load(Texture* texture);
bool unload(Texture* texture);

