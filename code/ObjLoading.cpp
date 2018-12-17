#include "ObjLoading.h"

#include "resource/ResourceManager.h"
#include "resource/resources/Mesh.h"
#include "resource/resources/Material.h"

#include "ecs/Ecs.h"
#include "ecs/components/TransformComponent.h"
#include "ecs/components/RenderComponent.h"
#include "ecs/components/ConvexHullColliderComponent.h"

#include "ConvexHull.h"
#include "Quickhull.h"

#include <fstream>
#include <sstream>
#include <cctype>

bool _loadObjInternal(FilenameString objFilename, Mesh* mesh, Ecs* ecs, bool createMeshes, bool createColliders)
{
    using namespace std;
    
    bool loadSubobjectsAsEntities = createMeshes || createColliders;
    bool considerMaterials = createMeshes || (!loadSubobjectsAsEntities && mesh->useMaterialsReferencedInObjFile);

    assert((mesh != nullptr) != loadSubobjectsAsEntities);
    assert(objFilename.substring(objFilename.length - 4) == ".obj");
    
    ifstream filestream(ResourceManager::instance().toFullPath(objFilename).cstr());

    vector<Vec3> v;    // vertices
    vector<Vec2> vt;   // uvs
    vector<Vec3> vn;   // normals

    FilenameString currentMaterialFilename = Material::DEFAULT_MATERIAL_FILENAME;
    MaterialNameString currentMaterialName = Material::DEFAULT_MATERIAL_NAME;
    unordered_map<string, Material*> materialsReferencedInObj;

    if (mesh) mesh->materialsReferencedInObjFile.clear();

    FilenameString relFileDirectory = truncateFilenameAfterDirectory(objFilename);

    string64 currentSubmeshName = "[unnamed]";
    vector<MeshVertex> currentSubmeshVertices; // @Slow. Can replace with hashset?
    vector<uint32> currentSubmeshIndices;

    vector<Vec3> currentHullVertices;

    uint32 unnamedSubmeshNameCount = 0;
    bool buildingFaces = false;

    bool eofFlush = false;
    string line;
    while (eofFlush || getline(filestream, line))
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

        for (uint32 i = 0; i < tokens.size(); i++)
        {
            // If there were multiple spaces in a row, using the ' ' tokenizer might still give us tokens with a whitespace value
            string token = tokens[i];
            bool isSpace = true;
            for (uint32 j = 0; j < token.length(); j++)
            {
                if (!std::isspace(token[j]))
                {
                    isSpace = false;
                    break;
                }
            }

            if (isSpace)
            {
                // ordered remove, fix up iteration
                tokens.erase(tokens.begin() + i);
                i--;
            }
        }

        if (tokens.size() == 0 && !eofFlush) tokens.push_back("#"); // @Hack: we may still want to flush a face (eof), so just treat it like a harmless character
        else if (tokens.size() > 0 && tokens[0] == "s") continue;

        if (buildingFaces && (eofFlush || tokens[0] != "f"))
        {
            buildingFaces = false;

            // Flush current mesh
            if (currentSubmeshName == "[unnamed]")
            {
                currentSubmeshName += std::to_string(unnamedSubmeshNameCount++).c_str();
            }

            Mesh* meshToPush = nullptr;
            if(createMeshes)
            {
                meshToPush = ResourceManager::instance().initMesh(objFilename, currentSubmeshName);
            }
            else if (!loadSubobjectsAsEntities)
            {
                meshToPush = mesh;
            }
            
            if (meshToPush)
            {
                // FLUSH current submesh
                meshToPush->submeshes.push_back(
                    Submesh(
                        meshToPush->id,
                        currentSubmeshName,
                        currentSubmeshVertices,
                        currentSubmeshIndices,
                        ResourceManager::instance().getMaterial(
                            (currentMaterialFilename != "" && currentMaterialName != "") ? currentMaterialFilename : Material::DEFAULT_MATERIAL_FILENAME,
                            (currentMaterialFilename != "" && currentMaterialName != "") ? currentMaterialName : Material::DEFAULT_MATERIAL_NAME
                        ),
                        meshToPush,
                        !loadSubobjectsAsEntities
                    )
                );

                if (loadSubobjectsAsEntities)
                {
                    meshToPush->useMaterialsReferencedInObjFile = true;
                    meshToPush->materialsReferencedInObjFile = std::move(materialsReferencedInObj);
                    materialsReferencedInObj.clear();
                }
            }
            
            if (loadSubobjectsAsEntities && (tokens.size() == 0 || tokens[0] != "usemtl")) // don't create the entity yet if it was just a material switch
            {
                Entity e = makeEntity(ecs, currentSubmeshName);

                // @Slow: constructing a convex hull only really needs to be done if we are creating colliders.
                //        Do it anyway to have a convenient way to approximate a centroid. A faster version would
                //        be to track positions of face centroids (and areas) as the faces are being built and then
                //        compute it.

                ConvexHull convHull;
                quickHull(currentHullVertices.data(), currentHullVertices.size(), &convHull, true); // if slow, try changing true to false

                Vec3 centroid = approximateHullCentroid(&convHull);
                TransformComponent* xfm = getComponent<TransformComponent>(e);
                xfm->setPosition(centroid);

                if (createMeshes)
                {
                    recalculatePositionsRelativeToCentroid(meshToPush, centroid);
                    recalculateBounds(meshToPush);
                    uploadToGpuOpenGl(meshToPush);
                    meshToPush->isLoaded = true;

                    for (uint32 i = 0; i < meshToPush->submeshes.size(); i++)
                    {
                        RenderComponent* rc = addComponent<RenderComponent>(e);
                        new (rc) RenderComponent(e, &meshToPush->submeshes[i]);
                    }
                }

                if (createColliders)
                {
                    recalculatePositionsRelativeToCentroid(&convHull, centroid);
                    ConvexHullColliderComponent* chcc = addComponent<ConvexHullColliderComponent>(e);
                    stdmoveConvexHullIntoComponent(chcc, &convHull);
                }

                currentHullVertices.clear();
            }

            currentSubmeshVertices.clear();
            currentSubmeshIndices.clear();
        }

        if (eofFlush)
        {
            // We finished flushing the final face after reaching end of file.
            break;
        }

        if (tokens[0] == "mtllib" && considerMaterials)
        {
            // material file
            currentMaterialFilename = relFileDirectory + tokens[1].c_str();
            currentMaterialName = "";
        }
        else if (tokens[0] == "o")
        {
            currentSubmeshName = tokens[1].c_str();

            if (loadSubobjectsAsEntities)
            {
                materialsReferencedInObj.clear();
            }
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
        else if (tokens[0] == "usemtl" && considerMaterials)
        {
            // init material in resource manager if it doesnt already exist.
            // do NOT load on init.
            currentMaterialName = tokens[1].c_str();

            Material* m = ResourceManager::instance().initMaterial(currentMaterialFilename, currentMaterialName, false);
            assert(m != nullptr);
            
            materialsReferencedInObj[m->id.cstr()] = m;
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
                    vector<Vec3> &currentHullVertices,
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
                            int v0pIndex = get<0>(vertexTuples[0]);
                            Vec3 v0 = objVertices[v0pIndex];
                            int v1pIndex = get<0>(vertexTuples[1]);
                            Vec3 v1 = objVertices[v1pIndex];
                            int v2pIndex = get<0>(vertexTuples[2]);
                            Vec3 v2 = objVertices[v2pIndex];

                            Vec3 v01 = v1 - v0;
                            Vec3 v02 = v2 - v0;

                            Vec3 computedFaceNormal = cross(v01, v02).normalizeOrXAxisInPlace();
                            assert(length(computedFaceNormal) > .99);
                            
                            vertex.normal = computedFaceNormal;
                        }

                        auto it = find(currentSubmeshVertices.begin(), currentSubmeshVertices.end(), vertex);
                        uint32 index;

                        if (it == currentSubmeshVertices.end())
                        {
                            currentSubmeshVertices.push_back(vertex);
                            index = currentSubmeshVertices.size() - 1;

                            if (find(currentHullVertices.begin(), currentHullVertices.end(), vertex.position) == currentHullVertices.end()) currentHullVertices.push_back(vertex.position);
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

            processFace(vertexTuples, currentSubmeshVertices, currentSubmeshIndices, currentHullVertices, v, vt, vn);
        }


        if (!eofFlush)
        {
            char peek = filestream.peek();
            if (peek == EOF) eofFlush = true;
        }
    }

    if (!loadSubobjectsAsEntities)
    {
        mesh->materialsReferencedInObjFile = std::move(materialsReferencedInObj);
    }

    return true;
}

bool loadObjIntoMesh(FilenameString objFile, Mesh* mesh)
{
    if (mesh->isLoaded) return true;
    
    return _loadObjInternal(objFile, mesh, nullptr, false, false);
}

bool loadObjSubobjectsAsEntities(FilenameString objFile, Ecs* ecs, bool createMeshes, bool createColliders)
{
    if (!ecs || (!createMeshes && !createColliders))
    {
        assert(false);
        return false;
    }
    
    return _loadObjInternal(objFile, nullptr, ecs, createMeshes, createColliders);
}
