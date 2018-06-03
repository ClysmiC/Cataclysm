#pragma once

#include <vector>
#include "Texture.h"
#include "MeshVertex.h"
#include "Submesh.h"
#include "CameraComponent.h"

#include "als_math.h"

struct Mesh
{
	Mesh() = default;
	Mesh(std::string filename, bool useMaterialsReferencedInObjFile);
	
    bool useMaterialsReferencedInObjFile;
	bool isLoaded = false;
	std::string id;
    std::vector<Submesh> submeshes;
};

bool load(Mesh* mesh);
bool unload(Mesh* mesh);

