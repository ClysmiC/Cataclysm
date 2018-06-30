#include "Mesh.h"
#include <fstream>
#include <sstream>
#include "assert.h"
#include <vector>
#include <unordered_map>
#include <tuple>
#include <unordered_set>
#include "ResourceManager.h"
#include <algorithm>

Mesh::Mesh(FilenameString filename, bool useMaterialsReferencedInObjFile_)
{
    this->id = filename;
    this->useMaterialsReferencedInObjFile = useMaterialsReferencedInObjFile_;
}

bool load(Mesh* mesh)
{
    using namespace std;

    if (mesh->isLoaded) return true;

    FilenameString filename = ResourceManager::instance().toFullPath(mesh->id);
    assert(filename.substring(filename.length - 4) == ".obj");

    ifstream objFile(filename.cstr());

    vector<Vec3> v;    // vertices
    vector<Vec2> vt;   // uvs
    vector<Vec3> vn;   // normals

    FilenameString currentMaterialFilename = Material::DEFAULT_MATERIAL_FILENAME;
    MaterialNameString currentMaterialName = Material::DEFAULT_MATERIAL_NAME;
    vector<Material*> materialsToLoad;

    FilenameString relFileDirectory = truncateFilenameAfterDirectory(mesh->id);
    
    string currentSubmeshName = "[unnamed]";
    vector<MeshVertex> currentSubmeshVertices;
    vector<uint32> currentSubmeshIndices;

    uint32 unnamedSubmeshNameCount = 0; // if submeshes aren't named, name them "submesh0", "submesh1", etc.
    bool buildingFaces = false;

    bool eofFlush = false;
    string line;
    while (eofFlush || getline(objFile, line))
    {
        if (eofFlush) line = "";

        istringstream ss(line);

        vector<string> tokens;
        {
            string item;
            while (getline(ss, item, ' '))
            {
                tokens.push_back(item);
            }
        }

        if (tokens.size() == 0 && !eofFlush) { tokens.push_back("#"); } // HACK: we may still want to flush a face, so just treat it like a harmless character

        if (buildingFaces && (eofFlush || tokens[0] != "f"))
        {
            buildingFaces = false;

            // Flush current mesh
            if (currentSubmeshName == "[unnamed]")
            {
                currentSubmeshName += unnamedSubmeshNameCount++;
            }

            // FLUSH current submesh
            mesh->submeshes.push_back(
                Submesh(
                    mesh->id,
                    currentSubmeshName.c_str(),
                    currentSubmeshVertices,
                    currentSubmeshIndices,
                    ResourceManager::instance().getMaterial(
                        (currentMaterialFilename != "" && currentMaterialName != "") ? currentMaterialFilename : Material::DEFAULT_MATERIAL_FILENAME,
                        (currentMaterialFilename != "" && currentMaterialName != "") ? currentMaterialName : Material::DEFAULT_MATERIAL_NAME
                    ),
                    mesh,
                    true
                )
            );

            currentSubmeshVertices.clear();
            currentSubmeshIndices.clear();

            currentSubmeshName = "[unnamed]";
        }

        if (eofFlush)
        {
            // We finished flushing the final material after reaching end of file.
            break;
        }

        if (tokens[0] == "mtllib" && mesh->useMaterialsReferencedInObjFile)
        {
            // material file
            currentMaterialFilename = relFileDirectory + tokens[1].c_str();
            currentMaterialName = "";
        }
        else if (tokens[0] == "o")
        {
            currentSubmeshName = tokens[1];
        }
        else if (tokens[0] == "v")
        {
            // vertex
            float32 x = stof(tokens[1]);
            float32 y = stof(tokens[2]);
            float32 z = stof(tokens[3]);
            v.push_back(Vec3(x, y, z));
        }
        else if (tokens[0] == "vt")
        {
            // tex coord
            float32 uCoord = stof(tokens[1]);
            float32 vCoord = stof(tokens[2]);
            vt.push_back(Vec2(uCoord, vCoord));
        }
        else if (tokens[0] == "vn")
        {
            // normal
            float32 x = stof(tokens[1]);
            float32 y = stof(tokens[2]);
            float32 z = stof(tokens[3]);
            vn.push_back(normalize(Vec3(x, y, z)));
        }
        else if (tokens[0] == "usemtl" && mesh->useMaterialsReferencedInObjFile)
        {
            // init material in resource manager if it doesnt already exist.
            // do NOT load on init.
            currentMaterialName = tokens[1].c_str();

            Material* m = ResourceManager::instance().initMaterial(currentMaterialFilename, currentMaterialName, false);
            assert(m != nullptr);

            materialsToLoad.push_back(m);
        }
        else if (tokens[0] == "f")
        {
            // face
            buildingFaces = true;

            // 2d array where each row is one point in a face and each column is one attribute of the point (position, uv, then normal)
            vector<tuple<int, int, int>> vertexTuples;

            for(uint32 i = 1; i < tokens.size(); i++)
            {
                vector<string> pointTokens;
                istringstream pointSs(tokens[i]);
                string token; 
                while (getline(pointSs, token, '/')) { pointTokens.push_back(token); }

                assert(pointTokens.size() >= 1);

                int vIndex = -1;
                int uvIndex = -1;
                int nIndex = -1;

                vIndex = stoi(pointTokens[0]) - 1;

                if (pointTokens.size() > 1 && pointTokens[1].length() > 0)
                {
                    uvIndex = stoi(pointTokens[1]) - 1;
                }

                if (pointTokens.size() > 2 && pointTokens[2].length() > 0)
                {
                    nIndex = stoi(pointTokens[2]) - 1;
                }
                
                tuple<int, int, int> point { vIndex, uvIndex, nIndex };
                vertexTuples.push_back(point);
            }

            // PROCESS FACE FUNCTOR
            struct {
                void operator() (
                    const vector<tuple<int, int, int>> &vertexTuples,
                    vector<MeshVertex> &currentSubmeshVertices,
                    vector<uint32> &currentSubmeshIndices,
                    const vector<Vec3> &objVertices,
                    const vector<Vec2> &objUvs,
                    const vector<Vec3> &objNormals
                    )
                {
                    assert(vertexTuples.size() >= 3);

                    uint32 anchorIndex;
                    uint32 mostRecentlyAddedIndex;
                    
                    for (uint32 i = 0; i < vertexTuples.size(); i++)
                    {
                        assert(vertexTuples.size() >= 3);
                        MeshVertex vertex;

                        bool computedFaceNormalSet = false;
                        Vec3 computedFaceNormal;

                        // 0-based indices
                        int pIndex, uvIndex, nIndex;
                        pIndex = get<0>(vertexTuples[i]);
                        uvIndex = get<1>(vertexTuples[i]);
                        nIndex = get<2>(vertexTuples[i]);

                        assert(pIndex >= 0);

                        vertex.position = objVertices[pIndex];
                        vertex.texCoords = (uvIndex < 0) ? Vec2(0, 0) : objUvs[uvIndex];

                        if (nIndex >= 0)
                        {
                            vertex.normal = objNormals[nIndex];
                        }
                        else
                        {
                            if (!computedFaceNormalSet)
                            {
                                int v0pIndex = get<0>(vertexTuples[0]);
                                Vec3 v0 = objVertices[v0pIndex];
                                int v1pIndex = get<0>(vertexTuples[1]);
                                Vec3 v1 = objVertices[v1pIndex];
                                int v2pIndex = get<0>(vertexTuples[2]);
                                Vec3 v2 = objVertices[v2pIndex];

                                Vec3 v01 = v1 - v0;
                                Vec3 v02 = v2 - v0;

                                computedFaceNormal = cross(v01, v02).normalizeOrXAxisInPlace();
                                assert(length(computedFaceNormal) > .99);
                                computedFaceNormalSet = true;
                            }
                            
                            vertex.normal = computedFaceNormal;
                        }

                        auto it = find(currentSubmeshVertices.begin(), currentSubmeshVertices.end(), vertex);
                        uint32 index;

                        if (it == currentSubmeshVertices.end())
                        {
                            currentSubmeshVertices.push_back(vertex);
                            index = currentSubmeshVertices.size() - 1;
                        }
                        else
                        {
                            index = it - currentSubmeshVertices.begin();
                        }

                        if (i == 0)
                        {
                            anchorIndex = index;
                        }

                        if (i >= 2)
                        {
                            currentSubmeshIndices.push_back(anchorIndex);
                            currentSubmeshIndices.push_back(mostRecentlyAddedIndex);
                            currentSubmeshIndices.push_back(index);
                        }

                        mostRecentlyAddedIndex = index;
                    }
                }
            } processFace;

            processFace(vertexTuples, currentSubmeshVertices, currentSubmeshIndices, v, vt, vn);
        }


        if (!eofFlush)
        {
            eofFlush = (objFile.peek() == EOF);
        }
    }

    // Load all materials that we discovered
    for (Material* material : materialsToLoad)
    {
        load(material);
    }

    //
    // Calculate bounds
    //
    {
        float32 minX = FLT_MAX;
        float32 minY = FLT_MAX;
        float32 minZ = FLT_MAX;

        float32 maxX = -FLT_MAX;
        float32 maxY = -FLT_MAX;
        float32 maxZ = -FLT_MAX;

        for (Submesh& submesh : mesh->submeshes)
        {
            Vec3 minPoint = submesh.bounds.center - submesh.bounds.halfDim;
            Vec3 maxPoint = submesh.bounds.center + submesh.bounds.halfDim;
            
            minX = std::min(minX, minPoint.x);
            minY = std::min(minY, minPoint.y);
            minZ = std::min(minZ, minPoint.z);
        
            maxX = std::max(maxX, maxPoint.x);
            maxY = std::max(maxY, maxPoint.y);
            maxZ = std::max(maxZ, maxPoint.z);
        }

        Vec3 minPoint = Vec3(minX, minY, minZ);
        Vec3 maxPoint = Vec3(maxX, maxY, maxZ);
        
        mesh->bounds.halfDim = Vec3(
            (maxPoint.x - minPoint.x) / 2.0f,
            (maxPoint.y - minPoint.y) / 2.0f,
            (maxPoint.z - minPoint.z) / 2.0f
        );
    
        mesh->bounds.center = minPoint + mesh->bounds.halfDim;
    }

    mesh->isLoaded = true;

    return true;
}

bool unload(Mesh* mesh)
{
    // TODO: what to do... remove self from resource manager?
    return false;
}

