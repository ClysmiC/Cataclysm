#include "Cubemap.h"

#include "ResourceManager.h"
#include "stb/stb_image.h"

#include "GL/glew.h"

bool GlobalCubemapInfo::isInited = false;
Shader *GlobalCubemapInfo::shader = nullptr;
uint32 GlobalCubemapInfo::vao = 0;
uint32 GlobalCubemapInfo::vbo = 0;
real32 GlobalCubemapInfo::vertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

void
GlobalCubemapInfo::init()
{
	if (isInited) return;

	isInited = true;

	ResourceManager& rm = ResourceManager::instance();
	shader = rm.initShader("shader/cubemap.vert", "shader/cubemap.frag", true);
	
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
}

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

void
Cubemap::render(CameraComponent* cameraComponent)
{
	Mat4 projection;
	projection.perspectiveInPlace(60.0f, 4.0f / 3.0f, 0.1f, 1000.0f);

	Mat4 view = cameraComponent->worldToViewMatrix();
	Mat4 viewProjectionSansTranslation = projection * view.mat3ifyInPlace();

	Shader *shader = GlobalCubemapInfo::shader;
	shader->bind();
			
	glActiveTexture(GL_TEXTURE0);
	shader->setInt("cubemap", 0);
	shader->setMat4("viewProjectionSansTranslation", viewProjectionSansTranslation);

	glDepthMask(GL_FALSE);
	glBindVertexArray(GlobalCubemapInfo::vao);
	glBindTexture(GL_TEXTURE_CUBE_MAP, openGlHandle);
			
	glDrawArrays(GL_TRIANGLES, 0, 36);
			
	glDepthMask(GL_TRUE);
}
