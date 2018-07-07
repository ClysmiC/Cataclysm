#include "TerrainComponent.h"
#include "ResourceManager.h"
#include "stb/stb_image.h"

TerrainComponent::TerrainComponent(FilenameString heightMapFile, Vec3 origin, float32 xLength, float32 zLength, float32 minHeight, float32 maxHeight)
    : mesh(heightMapFile, false)
{
    assert(minHeight <= maxHeight);
    
    this->origin = origin;
    this->xLength = xLength;
    this->zLength = zLength;
    this->minHeight = minHeight;
    this->maxHeight = maxHeight;

    int w, h, channels;
    FilenameString heightMapFullFilename = ResourceManager::instance().toFullPath(heightMapFile);
    unsigned char *data = stbi_load(heightMapFullFilename.cstr(), &w, &h, &channels, 1);

    assert(w > 1);
    assert(h > 1);

    this->xVertexCount = w;
    this->zVertexCount = h;

    std::vector<MeshVertex> vertices;
    vertices.reserve(this->xVertexCount * this->zVertexCount);

    std::vector<uint32> indices;
    indices.reserve((this->xVertexCount - 1) * (this->zVertexCount - 1) * 6);

    //
    // Fill out mesh vertices
    //
    {
        for (int down = 0; down < h; down++)
        {
            float32 downNormalized = ((float32)down) / (h - 1);
            float32 zPos = this->origin.z + zLength * downNormalized;
        
            for (int across = 0; across < w; across++)
            {
                float32 acrossNormalized = ((float32)across) / (w - 1);
                float32 xPos = this->origin.x + xLength * acrossNormalized;
            
                uint8 rawValue = (uint8)(data[down * w + across]);
                float32 normalizedValue = rawValue / 255.0f;
                float32 yPos = this->origin.y + minHeight + (maxHeight - minHeight) * normalizedValue;

                MeshVertex vertex;
                vertex.position = Vec3(xPos, yPos, zPos);
                vertex.texCoords = Vec2(acrossNormalized, downNormalized);
                vertices.push_back(vertex);
            }
        }
    }

    //
    // Fill out mesh indices
    //
    {
        for (int down = 0; down < h - 1; down++)
        {
            for (int across = 0; across < w - 1; across++)
            {
                uint32 topLeftIndex = down * w + across;
                uint32 topRightIndex = topLeftIndex + 1;
                uint32 botLeftIndex = (down + 1) * w + across;
                uint32 botRightIndex = botLeftIndex + 1;

                //  |\   //
                //  |_\  //
                indices.push_back(topLeftIndex);
                indices.push_back(botLeftIndex);
                indices.push_back(botRightIndex);

                //   _   //
                //  \ |  //
                //   \|  //
                indices.push_back(topLeftIndex);
                indices.push_back(botRightIndex);
                indices.push_back(topRightIndex);
            }
        }
    }
    
    //
    // Calculate normals
    //
    {
        struct RunningAverages
        {
            Vec3 normal;
            int samples;

            RunningAverages() { normal = Vec3(0, 1, 0); samples = 0; }

            void update(Vec3 n)
            {
                samples++;

                normal = ((samples - 1) / (float)samples) * normal + (1 / (float)samples) * n;
            }
        };

        std::unordered_map<uint32, RunningAverages> runningAverages;
        runningAverages.reserve(vertices.size());

        for (uint32 i = 0; i < indices.size(); i += 3)
        {
            uint32 aIndex = indices[i];
            uint32 bIndex = indices[i+1];
            uint32 cIndex = indices[i+2];

            MeshVertex& a = vertices.at(aIndex);
            MeshVertex& b = vertices.at(bIndex);
            MeshVertex& c = vertices.at(cIndex);

            Vec3 ab = b.position - a.position;
            Vec3 ac = c.position - a.position;

            Vec3 n = cross(ab, ac).normalizeInPlace();

            runningAverages[aIndex].update(n);
            runningAverages[bIndex].update(n);
            runningAverages[cIndex].update(n);
        }

        for (uint32 i = 0; i < vertices.size(); i++)
        {
            MeshVertex& mv = vertices[i];
            mv.normal = runningAverages[i].normal;
        }
    }

    this->mesh.submeshes.push_back(
        Submesh(
            heightMapFile,
            "terrain",
            vertices,
            indices,
            ResourceManager::instance().getMaterial(Material::DEFAULT_MATERIAL_FILENAME, Material::DEFAULT_MATERIAL_NAME),
            &(this->mesh)
        )
    );

    this->mesh.bounds = this->mesh.submeshes[0].bounds;
    this->mesh.isLoaded = true;
}
