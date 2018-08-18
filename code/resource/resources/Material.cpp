#include "Material.h"
#include "resource/ResourceManager.h"
#include "Texture.h"
#include "Shader.h"
#include <fstream>
#include <sstream>
#include "assert.h"
#include "als/als_math.h"

#include <GL/glew.h>

const char Material::COMPOSITE_ID_DELIMITER = '|';
const FilenameString Material::DEFAULT_MATERIAL_FILENAME = "default/default.mtl";
const MaterialNameString Material::DEFAULT_MATERIAL_NAME = "default";
const FilenameString Material::ERROR_MATERIAL_FILENAME = "default/error.mtl";
const MaterialNameString Material::ERROR_MATERIAL_NAME = "error";

Material::Material(FilenameString filename_, MaterialNameString materialName)
{
    this->filename = filename_;
    this->name = materialName;

    // Todo: grow fixed strings when RHS types and then only truncate once they are assigned to
    // the lhs type?
    this->id = filename;
    this->id += COMPOSITE_ID_DELIMITER;
    this->id += materialName;
    
    this->floatUniforms.emplace("material.specularExponent", 1);
    this->vec3Uniforms.emplace(
        "material.ambient",
        Vec3(0.5, 0.5, 0.5) 
    );
        
    this->vec3Uniforms.emplace(
        "material.diffuse",
        Vec3(0.5, 0.5, 0.5) 
    );
        
    this->vec3Uniforms.emplace(
        "material.specular",
        Vec3(0.5, 0.5, 0.5) 
    );

    this->textureUniforms.emplace("material.normalTex", Texture::defaultNormal());
    this->textureUniforms.emplace("material.ambientTex", Texture::gray());
    this->textureUniforms.emplace("material.diffuseTex", Texture::gray());
    this->textureUniforms.emplace("material.specularTex", Texture::black());
}

bool load(Material* material)
{
    if (material->isLoaded) return true;

    FilenameString fullFilename = ResourceManager::instance().toFullPath(material->filename);
    if (fullFilename.substring(fullFilename.length - 4) == ".mtl")
    {
        material->isLoaded = loadFromMtlFile(material, fullFilename);
    }
    else
    {
        material->isLoaded = true;
    }

    return material->isLoaded;
}

bool unload(Material* material)
{
    // TODO
    return false;
}

void clearUniforms(Material* material)
{
    material->boolUniforms.clear();
    material->intUniforms.clear();
    material->floatUniforms.clear();
    material->vec2Uniforms.clear();
    material->vec3Uniforms.clear();
    material->vec4Uniforms.clear();
    material->mat4Uniforms.clear();
    
    // Note: potential GPU memory leak
    material->textureUniforms.clear();
}

bool bind(Material* material)
{
    Shader* shader = material->shader;
    
    if (!material->isLoaded) return false;
    assert(shader != nullptr);
    if (shader == nullptr) return false;

    bool success = bind(shader);

    if (!success) return false;

    for (auto kvp : material->intUniforms)
    {
        setInt(shader, kvp.first, kvp.second);
    }

    for (auto kvp : material->boolUniforms)
    {
        setBool(shader, kvp.first, kvp.second);
    }

    for (auto kvp : material->floatUniforms)
    {
        setFloat(shader, kvp.first, kvp.second);
    }

    for (auto kvp : material->vec2Uniforms)
    {
        setVec2(shader, kvp.first, kvp.second);
    }

    for (auto kvp : material->vec3Uniforms)
    {
        setVec3(shader, kvp.first, kvp.second);
    }

    for (auto kvp : material->vec4Uniforms)
    {
        setVec4(shader, kvp.first, kvp.second);
    }

    for (auto kvp : material->mat4Uniforms)
    {
        setMat4(shader, kvp.first, kvp.second);
    }

    auto v = glGetError();

    uint32 textureUnit = GL_TEXTURE0;
    uint32 textureUnitIndex = 0;

    for (auto kvp : material->textureUniforms)
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, kvp.second->openGlHandle);
        setInt(shader, kvp.first, textureUnitIndex);
        
        textureUnit++;
        textureUnitIndex++;
    }

    return true;
}

bool loadFromMtlFile(Material* material, FilenameString fullFilename)
{
    // for every material declared in the .mtl file
    // if not inited, skip it
    // otherwise, get the reference to it and "load" it from within this function.
    // this should include loading ourself (assert at end to make sure this happens).

    // Thus, any subsequent material from within this file that gets "loaded" will just
    // return true because we already loaded them and set their isLoaded.
    
    using namespace std;

    struct
    {
        bool operator() (Material* m, string32 texType, FilenameString texFilename)
        {
            using namespace std;

            // Note: texFilename is relative to this material's directory, not the resource directory

            FilenameString relFileDirectory = truncateFilenameAfterDirectory(m->filename);
            FilenameString texRelFilename = relFileDirectory + texFilename;

            bool gammaCorrect = texType == "material.diffuseTex";
            Texture *tex = ResourceManager::instance().initTexture(texRelFilename, gammaCorrect, true);

            assert(tex != nullptr);
            if (tex == nullptr) return false;

            m->textureUniforms[texType] = tex;

            return true;
        }
    } handleTexture;

    bool foundSelfInMtlFile = false;

    ifstream mtlFile(fullFilename.cstr());

    Material *currentMaterial = nullptr;
    bool eofFlush = false;
    string line;

    while (eofFlush || getline(mtlFile, line))
    {
        if (eofFlush)
        {
            if (currentMaterial == nullptr)
            {
                // File didn't contain single material
                assert(false);
                break;
            }

            line = "";
        }

        istringstream ss(line);

        vector<string> tokens;
        string item;
        while (getline(ss, item, ' '))
        {
            tokens.push_back(item);
        }

        if (tokens.size() == 0 && !eofFlush) continue;

        if (eofFlush || tokens[0] == "newmtl")
        {
            if (currentMaterial != nullptr)
            {
                currentMaterial->shader = ResourceManager::instance().initShader("shader/basic.vert", "shader/basic.frag", true);;
                assert(currentMaterial->shader != nullptr);
                assert(currentMaterial->shader->isLoaded);

                currentMaterial->isLoaded = true;
            }

            if (!eofFlush)
            {
                currentMaterial = ResourceManager::instance().getMaterial(material->filename, tokens[1].c_str());

                if (currentMaterial == material) foundSelfInMtlFile = true;
            }
        }

        if (eofFlush)
        {
            // We finished flushing the final material after reaching end of file.
            break;
        }

        if (currentMaterial == nullptr)
        {
            continue;
        }

        if (tokens[0] == "Ns")
        {
            currentMaterial->floatUniforms["material.specularExponent"] = stof(tokens[1]);
        }
        else if (tokens[0] == "Ka")
        {
            currentMaterial->vec3Uniforms["material.ambient"] =
                Vec3(
                    stof(tokens[1]),
                    stof(tokens[2]),
                    stof(tokens[3]));
        }
        else if (tokens[0] == "Kd")
        {
            currentMaterial->vec3Uniforms["material.diffuse"] =
                Vec3(
                    stof(tokens[1]),
                    stof(tokens[2]),
                    stof(tokens[3]));
        }
        else if (tokens[0] == "Ks")
        {
            currentMaterial->vec3Uniforms["material.specular"] =
                Vec3(
                    stof(tokens[1]),
                    stof(tokens[2]),
                    stof(tokens[3]));
        }
        else if (tokens[0] == "illum")
        {
            // Todo:
            // Can I just ignore this and assume that if there is a
            // specular term that we should use specular lighting, etc.?
        }
        else if (tokens[0] == "map_Bump")
        {
            handleTexture(currentMaterial, "material.normalTex", tokens[1].c_str());
        }
        else if (tokens[0] == "map_Ka")
        {
            handleTexture(currentMaterial, "material.ambientTex", tokens[1].c_str());
        }
        else if (tokens[0] == "map_Kd")
        {
            handleTexture(currentMaterial, "material.diffuseTex", tokens[1].c_str());
        }
        else if (tokens[0] == "map_Ks")
        {
            handleTexture(currentMaterial, "material.specularTex", tokens[1].c_str());
        }
        else if (tokens[0] == "map_Ns")
        {
            // handleTexture(currentMaterial, "material.specularExponentTex", tokens[1]);
        }

        if (!eofFlush)
        {
            eofFlush = (mtlFile.peek() == EOF);
        }
    }

    assert(foundSelfInMtlFile); // TODO: replace this with error material somewhere
    if(!foundSelfInMtlFile)
    {
        // TODO: print or log some error... we are falling back to error material
    }
    
    return foundSelfInMtlFile;
}

