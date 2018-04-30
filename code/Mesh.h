#pragma once

#include <vector>
#include "Texture.h"
#include "MeshVertex.h"
#include "Submesh.h"

#include "als_math.h"

struct Mesh
{
	void init(const std::string filename, bool useMaterialsReferencedInObjFile);
	
    bool useMaterialsReferencedInObjFile;
    bool load();
    bool unload();
    void draw(const Mat4 &transform);

	bool isLoaded;
	std::string id;
    std::vector<Submesh> submeshes;
};

