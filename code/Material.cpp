#include "Material.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include "assert.h"
#include "als_math.h"

#include <GL/glew.h>

const std::string Material::COMPOSITE_ID_DELIMITER = "|";
const std::string Material::DEFAULT_MATERIAL_FILENAME = "default/default.mtl";
const std::string Material::DEFAULT_MATERIAL_NAME = "default";

void Material::init(std::string filename_, std::string materialName)
{
	this->filename = filename_;
	this->name = materialName;
	this->id = filename + COMPOSITE_ID_DELIMITER + materialName;
	
	floatUniforms.emplace("material.specularExponent", 1);
	vec3Uniforms.emplace(
		"material.ambient",
		Vec3(0.5, 0.5, 0.5) 
	);
		
	vec3Uniforms.emplace(
		"material.diffuse",
		Vec3(0.5, 0.5, 0.5) 
	);
		
	vec3Uniforms.emplace(
		"material.specular",
		Vec3(0.5, 0.5, 0.5) 
	);

	textureUniforms.emplace("material.normalTex", Texture::defaultNormal());
	textureUniforms.emplace("material.ambientTex", Texture::gray());
	textureUniforms.emplace("material.diffuseTex", Texture::gray());
	textureUniforms.emplace("material.specularTex", Texture::black());
}

bool Material::load()
{
    if (isLoaded) return true;

    std::string fullFilename = ResourceManager::instance().toFullPath(filename);
    if (fullFilename.substr(fullFilename.length() - 4) == ".mtl")
    {
        isLoaded = loadFromMtlFile(fullFilename);
    }
    else
    {
        isLoaded = true;
    }

    return isLoaded;
}

bool Material::unload()
{
    return false;
}

void Material::clearUniforms()
{
    boolUniforms.clear();
    intUniforms.clear();
    floatUniforms.clear();
    vec2Uniforms.clear();
    vec3Uniforms.clear();
    vec4Uniforms.clear();
	mat4Uniforms.clear();
	
    // Note: potential memory leak
    textureUniforms.clear();
}

bool Material::bind()
{
    if (!isLoaded) return false;
    assert(shader != nullptr);
    if (shader == nullptr) return false;

    bool success = shader->bind();

    if (!success) return false;

    for (auto kvp : intUniforms)
    {
        shader->setInt(kvp.first, kvp.second);
    }

    for (auto kvp : boolUniforms)
    {
        shader->setBool(kvp.first, kvp.second);
    }

    for (auto kvp : floatUniforms)
    {
        shader->setFloat(kvp.first, kvp.second);
    }

    for (auto kvp : vec2Uniforms)
    {
        shader->setVec2(kvp.first, kvp.second);
    }

    for (auto kvp : vec3Uniforms)
    {
        shader->setVec3(kvp.first, kvp.second);
    }

    for (auto kvp : vec4Uniforms)
    {
        shader->setVec4(kvp.first, kvp.second);
    }

    for (auto kvp : mat4Uniforms)
    {
        shader->setMat4(kvp.first, kvp.second);
    }

    auto v = glGetError();

    uint32 textureUnit = GL_TEXTURE0;
    uint32 textureUnitIndex = 0;

    for (auto kvp : textureUniforms)
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, kvp.second->openGlHandle);
        shader->setInt(kvp.first, textureUnitIndex);

        textureUnit++;
        textureUnitIndex++;
    }

    return true;
}

bool Material::loadFromMtlFile(std::string fullFilename)
{
    using namespace std;

    bool foundSelfInMtlFile = false;

    ifstream mtlFile(fullFilename);

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
                bool success = currentMaterial->setupShader();
                assert(success);

                currentMaterial->isLoaded = true;
            }

            if (!eofFlush)
            {
                currentMaterial = ResourceManager::instance().getMaterial(filename, tokens[1]);

                if (currentMaterial == this) foundSelfInMtlFile = true;
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
            handleTexture(*currentMaterial, "material.normalTex", tokens[1]);
        }
        else if (tokens[0] == "map_Ka")
        {
            handleTexture(*currentMaterial, "material.ambientTex", tokens[1]);
        }
        else if (tokens[0] == "map_Kd")
        {
            handleTexture(*currentMaterial, "material.diffuseTex", tokens[1]);
        }
        else if (tokens[0] == "map_Ks")
        {
            handleTexture(*currentMaterial, "material.specularTex", tokens[1]);
        }
        else if (tokens[0] == "map_Ns")
        {
            // handleTexture(*currentMaterial, "material.specularExponentTex", tokens[1]);
        }

        if (!eofFlush)
        {
            eofFlush = (mtlFile.peek() == EOF);
        }
    }

    // for every material declared in the .mtl file
    // if not inited, skip it
    // otherwise, get the reference to it and "load" it from within this function.
    // this should include loading ourself (add assert to make sure this happens).

    // Thus, any subsequent material from within this file that gets "loaded" will just
    // return true because we already loaded them and set their isLoaded.

    assert(foundSelfInMtlFile);
    return foundSelfInMtlFile;
}

bool Material::handleTexture(Material& material, std::string texType, std::string texFilename)
{
    using namespace std;

    // Note: texFilename is relative to this material's directory, not the resource directory

    string relFileDirectory = truncateFilenameAfterDirectory(filename);
    string texRelFilename = relFileDirectory + texFilename;

    bool gammaCorrect = texType == "material.diffuseTex";
    Texture *tex = ResourceManager::instance().initTexture(texRelFilename, gammaCorrect, true);

    assert(tex != nullptr);
    if (tex == nullptr) return false;

    material.textureUniforms[texType] = tex;

    return true;
}

bool Material::setupShader()
{
    ResourceManager::instance().initShader("shader/basic.vert", "shader/basic.frag", false);
    Shader* s = ResourceManager::instance().getShader("shader/basic.vert", "shader/basic.frag");
    shader = s;
    return s->load();
}
