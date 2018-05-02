#include "Mesh.h"
#include <fstream>
#include <sstream>
#include "assert.h"
#include <vector>
#include <unordered_map>
#include <tuple>
#include <unordered_set>
#include "ResourceManager.h"

void Mesh::init(const std::string filename, bool useMaterialsReferencedInObjFile_)
{
	this->id = filename;
	this->useMaterialsReferencedInObjFile = useMaterialsReferencedInObjFile_;
}

bool Mesh::load()
{
    using namespace std;

    string filename = ResourceManager::instance().toFullPath(id);
    assert(filename.substr(filename.length() - 4) == ".obj");

    ifstream objFile(filename);

    vector<Vec3> v;    // vertices
    vector<Vec2> vt;   // uvs
    vector<Vec3> vn;   // normals

    string currentMaterialFilename = Material::DEFAULT_MATERIAL_FILENAME;
    string currentMaterialName = Material::DEFAULT_MATERIAL_NAME;
    vector<Material*> materialsToLoad;

    string relFileDirectory = truncateFilenameAfterDirectory(id);
    
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

            // submesh
            assert(currentMaterialName != "");

            // FLUSH current submesh
            submeshes.push_back(
                Submesh(
                    id,
                    currentSubmeshName,
                    currentSubmeshVertices,
                    currentSubmeshIndices,
                    ResourceManager::instance().getMaterial(currentMaterialFilename, currentMaterialName)));

            currentSubmeshVertices.clear();
            currentSubmeshIndices.clear();
            // v.clear();
            // vt.clear();
            // vn.clear();

            currentSubmeshName = "[unnamed]";
        }

        if (eofFlush)
        {
            // We finished flushing the final material after reaching end of file.
            break;
        }

        if (tokens[0] == "mtllib" && useMaterialsReferencedInObjFile)
        {
            // material file
            currentMaterialFilename = relFileDirectory + tokens[1];
            currentMaterialName = "";
        }
        else if (tokens[0] == "o")
        {
            currentSubmeshName = tokens[1];
        }
        else if (tokens[0] == "v")
        {
            // vertex
            real32 x = stof(tokens[1]);
            real32 y = stof(tokens[2]);
            real32 z = stof(tokens[3]);
            v.push_back(Vec3(x, y, z));
        }
        else if (tokens[0] == "vt")
        {
            // tex coord
            real32 uCoord = stof(tokens[1]);
            real32 vCoord = stof(tokens[2]);
            vt.push_back(Vec2(uCoord, vCoord));
        }
        else if (tokens[0] == "vn")
        {
            // normal
            real32 x = stof(tokens[1]);
            real32 y = stof(tokens[2]);
            real32 z = stof(tokens[3]);
            vn.push_back(normalize(Vec3(x, y, z)));
        }
        else if (tokens[0] == "usemtl" && useMaterialsReferencedInObjFile)
        {
            // init material in resource manager if it doesnt already exist.
            // do NOT load on init.
            currentMaterialName = tokens[1];

            Material* m = ResourceManager::instance().initMaterial(currentMaterialFilename, currentMaterialName, false);
            assert(m != nullptr);

            materialsToLoad.push_back(m);
        }
        else if (tokens[0] == "f")
        {
            // face
            buildingFaces = true;

            vector<string> faceTokens;
            {
                string item;
                istringstream ss1(tokens[1]);
                istringstream ss2(tokens[2]);
                istringstream ss3(tokens[3]);
                while (getline(ss1, item, '/')) { faceTokens.push_back(item); }
                while (getline(ss2, item, '/')) { faceTokens.push_back(item); }
                while (getline(ss3, item, '/')) { faceTokens.push_back(item); }
            }

            // If UV's are omitted, set them to 0
            if (faceTokens[1].size() == 0) { faceTokens[1] = "-1"; }
            if (faceTokens[4].size() == 0) { faceTokens[4] = "-1"; }
            if (faceTokens[7].size() == 0) { faceTokens[7] = "-1"; }

            tuple<int, int, int> aTuple{ stoi(faceTokens[0]) - 1, stoi(faceTokens[1]) - 1, stoi(faceTokens[2]) - 1 };
            tuple<int, int, int> bTuple{ stoi(faceTokens[3]) - 1, stoi(faceTokens[4]) - 1, stoi(faceTokens[5]) - 1 };
            tuple<int, int, int> cTuple{ stoi(faceTokens[6]) - 1, stoi(faceTokens[7]) - 1, stoi(faceTokens[8]) - 1 };

            // PROCESS VERTEX FUNCTOR
            struct {
                void operator() (
                    const tuple<int, int, int> &vertexTuple,
                    vector<MeshVertex> &currentSubmeshVertices,
                    vector<uint32> &currentSubmeshIndices,
                    const vector<Vec3> &objVertices,
                    const vector<Vec2> &objUvs,
                    const vector<Vec3> &objNormals
                    )
                {
                    MeshVertex vertex;

                    // 0-based indices
                    int pIndex, uvIndex, nIndex;
                    pIndex = get<0>(vertexTuple);
                    uvIndex = get<1>(vertexTuple);
                    nIndex = get<2>(vertexTuple);

                    assert(pIndex >= 0);
                    assert(nIndex >= 0);

                    vertex.position = objVertices[pIndex];
                    vertex.texCoords = (uvIndex < 0) ? Vec2(0, 0) : objUvs[uvIndex];
                    vertex.normal = objNormals[nIndex];

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

                    currentSubmeshIndices.push_back(index);
                }
            } processVertex;

            processVertex(aTuple, currentSubmeshVertices, currentSubmeshIndices, v, vt, vn);
            processVertex(bTuple, currentSubmeshVertices, currentSubmeshIndices, v, vt, vn);
            processVertex(cTuple, currentSubmeshVertices, currentSubmeshIndices, v, vt, vn);
        }


        if (!eofFlush)
        {
            eofFlush = (objFile.peek() == EOF);
        }
    }

    // Load all materials that we discovered
    for (Material* material : materialsToLoad)
    {
        material->load();
    }

    isLoaded = true;

    return true;
}

bool Mesh::unload()
{
    // TODO: what to do... remove self from resource manager?
    isLoaded = false;
    return true;
}

void Mesh::draw(const Mat4 &transform, Camera camera)
{
    for (Submesh s : submeshes)
    {
        s.draw(transform, camera);
    }
}
