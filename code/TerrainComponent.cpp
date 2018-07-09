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
            std::vector<uint32> indices;

            //
            // Fill out mesh vertices
            //

            // Even though the chunk only owns a certain # of vertices,
            // we may need to render 1 extra vertex to stitch it together
            // to an adjacent chunk
            uint32 xVerticesToRender = this->xVerticesPerChunk;
            uint32 zVerticesToRender = this->zVerticesPerChunk;
            
            vertices.reserve(xVerticesToRender * zVerticesToRender);
            indices.reserve(xVerticesToRender * zVerticesToRender * 6);

            if (chunkAcross < xChunksNeeded - 1)
            {
                chunk->hasExtraXVertex = true;
                xVerticesToRender++;
            }
            
            if (chunkDown   < zChunksNeeded - 1)
            {
                chunk->hasExtraZVertex = true;
                zVerticesToRender++;
            }
            
            {
                for (uint32 down = 0; down < zVerticesToRender; down++)
                {
                    uint32 downPixel = this->zVerticesPerChunk * chunkDown + down;
                    uint32 downPixelClamped = downPixel;
                    if (downPixelClamped > imgHeight - 1) downPixelClamped = imgHeight - 1;
                    
                    float32 zPos = this->origin.z + downPixel * deltaZPerVertex;
        
                    for (uint32 across = 0; across < xVerticesToRender; across++)
                    {
                        uint32 acrossPixel = this->xVerticesPerChunk * chunkAcross + across;
                        uint32 acrossPixelClamped = acrossPixel;
                        if (acrossPixelClamped > imgWidth - 1) acrossPixelClamped = imgWidth - 1;
                        
                        float32 xPos = this->origin.x + acrossPixel * deltaXPerVertex;

                        maxXDebug = fmax(maxXDebug, xPos);
            
                        uint8 rawValue = (uint8)(imgData[downPixelClamped * imgWidth + acrossPixelClamped]);
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

                    assert(isNormal(n));
                    
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

    //
    // Free image data
    //
    stbi_image_free(imgData);

    //
    // Touch up the normals at the seams
    //
    for (uint32 chunkDown = 0; chunkDown < zChunksNeeded; chunkDown++)
    {
        for (uint32 chunkAcross = 0; chunkAcross < xChunksNeeded; chunkAcross++)
        {
            TerrainChunk* chunk = &this->chunks[chunkDown][chunkAcross];
            
            uint32 xVerticesInMesh = this->xVerticesPerChunk + (chunk->hasExtraXVertex ? 1 : 0);
            uint32 zVerticesInMesh = this->zVerticesPerChunk + (chunk->hasExtraZVertex ? 1 : 0);

            bool isTopmostChunk = chunkDown == 0;
            bool isLeftmostChunk = chunkAcross == 0;

            // Fix horizontal seam
            if (chunkDown < zChunksNeeded - 1)
            {
                TerrainChunk* chunkBelow = &this->chunks[chunkDown + 1][chunkAcross];
                for (uint32 i = 0; i < xVerticesInMesh - 1; i++)
                {
                    // The mesh to our left will handle this one as its corner fix-up
                    if (i == 0 && !isLeftmostChunk) continue;
                    
                    MeshVertex& v1 = chunk->mesh.submeshes[0].vertices.at(
                        xVerticesInMesh * (zVerticesInMesh - 1) + i
                    );

                    MeshVertex& v2 = chunkBelow->mesh.submeshes[0].vertices.at(
                        i
                    );

                    Vec3 normal = normalize((v1.normal + v2.normal) / 2);

                    v1.normal = normal;
                    v2.normal = normal;
                }
            }

            // Fix vertical seam
            if (chunkAcross < xChunksNeeded - 1)
            {
               TerrainChunk* chunkToRight = &this->chunks[chunkDown][chunkAcross + 1];
               
               uint32 xVerticesInMeshToRight = this->xVerticesPerChunk + (chunkToRight->hasExtraXVertex ? 1 : 0);

               for (uint32 i = 0; i < this->zVerticesPerChunk - 1; i++)
               {
                   // The mesh to our top will handle this one as its corner fix-up
                   if (i == 0 && !isTopmostChunk) continue;
                   
                   MeshVertex& v1 = chunk->mesh.submeshes[0].vertices.at(
                       xVerticesInMesh * i + xVerticesInMesh - 1
                   );

                   MeshVertex& v2 = chunkToRight->mesh.submeshes[0].vertices.at(
                       xVerticesInMeshToRight * i
                   );

                   Vec3 normal = normalize((v1.normal + v2.normal) / 2);

                   v1.normal = normal;
                   v2.normal = normal;
               }
            }

            // Corner fix-up
            if (chunkDown < zChunksNeeded - 1 && chunkAcross < xChunksNeeded - 1)
            {
                TerrainChunk* chunkBelow = &this->chunks[chunkDown + 1][chunkAcross];
                TerrainChunk* chunkToRight = &this->chunks[chunkDown][chunkAcross + 1];
                TerrainChunk* chunkDiagonal = &this->chunks[chunkDown + 1][chunkAcross + 1];

                uint32 xVerticesInMeshToRight = this->xVerticesPerChunk + (chunkToRight->hasExtraXVertex ? 1 : 0);

                // Bot right point
                MeshVertex& v1 = chunk->mesh.submeshes[0].vertices.at(
                    xVerticesInMesh * zVerticesInMesh - 1
                );

                // Bot left point of chunk to right
                MeshVertex& v2 = chunkToRight->mesh.submeshes[0].vertices.at(
                    xVerticesInMeshToRight * (zVerticesInMesh - 1)
                );

                // Top right point of chunk below
                MeshVertex& v3 = chunkBelow->mesh.submeshes[0].vertices.at(
                    xVerticesInMesh - 1
                );

                // Top left point of chunk diagonal
                MeshVertex& v4 = chunkDiagonal->mesh.submeshes[0].vertices.at(
                    0
                );

                Vec3 normal = normalize((v1.normal + v2.normal + v3.normal + v4.normal) / 4);

                v1.normal = normal;
                v2.normal = normal;
                v3.normal = normal;
                v4.normal = normal;
            }

            // Don't need to re-upload the neighboring chunks that we modified, as they will
            // upload themselves on their turn in the iteration loop
            reuploadModifiedVerticesToGpu(&chunk->mesh.submeshes[0]);
        }
    }
}
