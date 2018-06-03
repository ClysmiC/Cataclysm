#include "Material.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include "assert.h"
#include "als_math.h"

#include <GL/glew.h>
#include "als.h"

const std::string Material::COMPOSITE_ID_DELIMITER = "|";
const std::string Material::DEFAULT_MATERIAL_FILENAME = "default/default.mtl";
const std::string Material::DEFAULT_MATERIAL_NAME = "default";

Material::Material(std::string filename_, std::string materialName)
{
	this->filename = filename_;
	this->name = materialName;
	this->id = filename + COMPOSITE_ID_DELIMITER + materialName;
	
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

    std::string fullFilename = ResourceManager::instance().toFullPath(material->filename);
    if (fullFilename.substr(fullFilename.length() - 4) == ".mtl")
    {
        isLoaded = loadFromMtlFile(material, fullFilename);
    }
    else
    {
        isLoaded = true;
    }

    return isLoaded;
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
	
    if (!isLoaded) return false;
    assert(shader != nullptr);
    if (shader == nullptr) return false;

    bool success = shader->bind();

    if (!success) return false;

    for (auto kvp : material->intUniforms)
    {
        shader->setInt(kvp.first, kvp.second);
    }

    for (auto kvp : material->boolUniforms)
    {
        shader->setBool(kvp.first, kvp.second);
    }

    for (auto kvp : material->floatUniforms)
    {
        shader->setFloat(kvp.first, kvp.second);
    }

    for (auto kvp : material->vec2Uniforms)
    {
        shader->setVec2(kvp.first, kvp.second);
    }

    for (auto kvp : material->vec3Uniforms)
    {
        shader->setVec3(kvp.first, kvp.second);
    }

    for (auto kvp : material->vec4Uniforms)
    {
        shader->setVec4(kvp.first, kvp.second);
    }

    for (auto kvp : material->mat4Uniforms)
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

bool loadFromMtlFile(Material* material, std::string fullFilename)
{
	// for every material declared in the .mtl file
    // if not inited, skip it
    // otherwise, get the reference to it and "load" it from within this function.
    // this should include loading ourself (assert at end to make sure this happens).

    // Thus, any subsequent material from within this file that gets "loaded" will just
    // return true because we already loaded them and set their isLoaded.
	
    using namespace std;

	lambda handleTexture = [](Material* m, std::string texType, std::string texFilename) -> bool
	{
		using namespace std;

		// Note: texFilename is relative to this material's directory, not the resource directory

		string relFileDirectory = truncateFilenameAfterDirectory(filename);
		string texRelFilename = relFileDirectory + texFilename;

		bool gammaCorrect = texType == "material.diffuseTex";
		Texture *tex = ResourceManager::instance().initTexture(texRelFilename, gammaCorrect, true);

		assert(tex != nullptr);
		if (tex == nullptr) return false;

		m->textureUniforms[texType] = tex;

		return true;
	}

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
                bool success = ResourceManager::instance().initShader("shader/basic.vert", "shader/basic.frag", true);;
                assert(success);

                currentMaterial->isLoaded = true;
            }

            if (!eofFlush)
            {
                currentMaterial = ResourceManager::instance().getMaterial(material->filename, tokens[1]);

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

    assert(foundSelfInMtlFile);
    return foundSelfInMtlFile;
}

