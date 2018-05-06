#pragma once

#include <vector>
#include "Texture.h"
#include "MeshVertex.h"
#include "Submesh.h"
#include "Camera.h"

#include "als_math.h"

struct Mesh
{
	void init(const std::string filename, bool useMaterialsReferencedInObjFile);
	
    bool useMaterialsReferencedInObjFile;
    bool load();
    bool unload();
    // void draw(const Mat4 &transform, Camera camera);

	bool isLoaded = false;
	std::string id;
    std::vector<Submesh> submeshes;
};

