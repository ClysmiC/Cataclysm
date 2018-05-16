#include "Cubemap.h"

#include "ResourceManager.h"
#include "stb/stb_image.h"

#include "GL/glew.h"

void
Cubemap::init(const std::string& directory, const std::string& extension_)
{
	this->id = directory;
	this->extension = extension_;
}

bool
Cubemap::load()
{
	using namespace std;
	
	if (isLoaded) return true;

	glGenTextures(1, &openGlHandle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, openGlHandle);

	std::string filenames[] = { "right", "left", "top", "bottom", "front", "back" };

    stbi_set_flip_vertically_on_load(false);
	GLuint glTexTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	
	for (std::string filename : filenames)
	{
		std::string fullName = ResourceManager::instance().toFullPath(this->id + "/" + filename) + this->extension;
		
		int w = 0, h = 0, channels = 0;
		unsigned char *data = stbi_load(fullName.c_str(), &w, &h, &channels, 0);

		assert(w > 0);

		glTexImage2D(glTexTarget, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);

		glTexTarget++;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
    isLoaded = true;
    return true;
}

bool
Cubemap::unload()
{
	assert(isLoaded);

    // Unload texture from OpenGL
    glDeleteTextures(1, &openGlHandle);

    isLoaded = false;

    return true;
}
