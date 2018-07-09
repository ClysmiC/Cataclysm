#include "TerrainComponent.h"
#include "ResourceManager.h"
#include "stb/stb_image.h"

TerrainComponent::TerrainComponent(FilenameString heightMapFile, Vec3 origin, float32 xLength, float32 zLength, float32 minHeight, float32 maxHeight)
{
    assert(minHeight <= maxHeight);

    this->origin = origin;

    unsigned char *imgData;
    uint32 imgWidth;
    uint32 imgHeight;

    //
    // Read in image
    //
    {
        int w, h, channels;
        FilenameString heightMapFullFilename = ResourceManager::instance().toFullPath(heightMapFile);
        imgData = stbi_load(heightMapFullFilename.cstr(), &w, &h, &channels, 1);

        assert(w > 1);
        assert(h > 1);

        imgWidth = w;
        imgHeight = h;
    }

    uint32 xChunksNeeded = (imgWidth + this->xVerticesPerChunk - 1) / this->xVerticesPerChunk;
    uint32 zChunksNeeded = (imgHeight + this->zVerticesPerChunk - 1) / this->zVerticesPerChunk;
    
    this->xLengthPerChunk = xLength / (float32)xChunksNeeded;
    this->zLengthPerChunk = zLength / (float32)zChunksNeeded;
    this->xChunkCount = xChunksNeeded;
    this->zChunkCount = zChunksNeeded;

    float32 deltaXPerVertex = xLength / (xChunksNeeded * this->xVerticesPerChunk - 1);
    float32 deltaZPerVertex = zLength / (xChunksNeeded * this->zVerticesPerChunk - 1);

    this->chunks.reserve(zChunksNeeded);

    float32 maxXDebug = 0;

    for (uint32 chunkDown = 0; chunkDown < zChunksNeeded; chunkDown++)
    {
        this->chunks.push_back(std::vector<TerrainChunk>());
        this->chunks[chunkDown].reserve(xChunksNeeded);

        for (uint32 chunkAcross = 0; chunkAcross < xChunksNeeded; chunkAcross++)
        {
            this->chunks[chunkDown].push_back(TerrainChunk());
            TerrainChunk* chunk = &this->chunks[chunkDown][chunkAcross];
            chunk->minHeight = minHeight;
            chunk->maxHeight = maxHeight;
            chunk->terrainComponent = this;
            chunk->xIndex = chunkAcross;
            chunk->zIndex = chunkDown;
            chunk->mesh.useMaterialsReferencedInObjFile = false;
            chunk->mesh.id = "terrainMesh"; // This isn't unique, but since it isn't managed by resource manager it should be okay
            
            std::vector<MeshVertex> vertices;
            vertices.reserve(this->xVerticesPerChunk * this->zVerticesPerChunk);

            std::vector<uint32> indices;
            indices.reserve((this->xVerticesPerChunk - 1) * (this->zVerticesPerChunk - 1) * 6);

            //
            // Fill out mesh vertices
            //

            // Even though the chunk only owns a certain # of vertices,
            // we may need to render 1 extra vertex to stitch it together
            // to an adjacent chunk
            uint32 xVerticesToRender = this->xVerticesPerChunk;
            uint32 zVerticesToRender = this->zVerticesPerChunk;

            if (chunkAcross < xChunksNeeded - 1) xVerticesToRender++;
            if (chunkDown   < zChunksNeeded - 1) zVerticesToRender++;
            
            {
                for (uint32 down = 0; down < zVerticesToRender; down++)
                {
                    uint32 downPixel = this->zVerticesPerChunk * chunkDown + down;
                    if (downPixel > imgHeight - 1) downPixel = imgHeight - 1;
                    
                    float32 zPos = this->origin.z + downPixel * deltaZPerVertex;
        
                    for (uint32 across = 0; across < xVerticesToRender; across++)
                    {
                        uint32 acrossPixel = this->xVerticesPerChunk * chunkAcross + across;
                        if (acrossPixel > imgWidth - 1) acrossPixel = imgWidth - 1;
                        
                        float32 xPos = this->origin.x + acrossPixel * deltaXPerVertex;

                        maxXDebug = fmax(maxXDebug, xPos);
            
                        uint8 rawValue = (uint8)(imgData[downPixel * imgWidth + acrossPixel]);
                        float32 normalizedValue = rawValue / 255.0f;

                        float32 yPos = this->origin.y + chunk->minHeight + (chunk->maxHeight - chunk->minHeight) * normalizedValue;

                        MeshVertex vertex;
                        vertex.position = Vec3(xPos, yPos, zPos);
                        vertex.texCoords = Vec2(acrossPixel / ((float32)imgWidth - 1), downPixel / ((float32)imgHeight - 1));
                        vertices.push_back(vertex);
                    }
                }
            }

            //
            // Fill out mesh indices
            //
            {
                for (uint32 down = 0; down < zVerticesToRender - 1; down++)
                {
                    for (uint32 across = 0; across < xVerticesToRender - 1; across++)
                    {
                        uint32 topLeftIndex = down * xVerticesToRender + across;
                        uint32 topRightIndex = topLeftIndex + 1;
                        uint32 botLeftIndex = (down + 1) * xVerticesToRender + across;
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
                    mv.normal = normalize(runningAverages[i].normal);
                }
            }

            chunk->mesh.submeshes.push_back(
                Submesh(
                    heightMapFile,
                    "terrain",
                    vertices,
                    indices,
                    ResourceManager::instance().getMaterial(Material::DEFAULT_MATERIAL_FILENAME, Material::DEFAULT_MATERIAL_NAME),
                    &(chunk->mesh)
                )
            );

            chunk->mesh.bounds = chunk->mesh.submeshes[0].bounds;
            chunk->mesh.isLoaded = true;
        }
    }

    stbi_image_free(imgData);
}
