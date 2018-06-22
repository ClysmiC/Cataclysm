#include "Cubemap.h"

#include "ResourceManager.h"
#include "stb/stb_image.h"

#include "Shader.h"
#include "TransformComponent.h"
#include "CameraComponent.h"

#include "GL/glew.h"

Cubemap::Cubemap(std::string directory, std::string extension_)
{
    this->id = directory;
    this->extension = extension_;
}

uint32 cubemapVao()
{
    static uint32 vao;

    if (vao == 0)
    {
        uint32 vbo;

        real32 vertices[] = {
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
        
        glGenBuffers(1, &vbo);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
    }

    return vao;
}

Shader* cubemapShader()
{
    static Shader* theShader = nullptr;

    if (theShader == nullptr)
    {
        ResourceManager& rm = ResourceManager::instance();
        theShader = rm.initShader("shader/cubemap.vert", "shader/cubemap.frag", true);
    }

    return theShader;
}

bool load(Cubemap* cubemap)
{
    using namespace std;
    
    if (cubemap->isLoaded) return true;

    glGenTextures(1, &cubemap->openGlHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->openGlHandle);

    std::string filenames[] = { "right", "left", "top", "bottom", "front", "back" };

    stbi_set_flip_vertically_on_load(false);
    GLuint glTexTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    
    for (std::string filename : filenames)
    {
        std::string fullName = ResourceManager::instance().toFullPath(cubemap->id + "/" + filename) + cubemap->extension;
        
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
    
    cubemap->isLoaded = true;
    return true;
}

bool unload(Cubemap* cubemap)
{
    assert(cubemap->isLoaded);

    // Unload texture from OpenGL
    glDeleteTextures(1, &cubemap->openGlHandle);

    cubemap->isLoaded = false;

    return true;
}

void renderCubemap(Cubemap* cubemap, CameraComponent* camera, Transform* cameraXfm)
{
    Mat4 w2v = worldToView(cameraXfm);
    Mat4 viewProjectionSansTranslation = camera->projectionMatrix * w2v.mat3ifyInPlace();

    Shader *s = cubemapShader();
    bind(s);
            
    glActiveTexture(GL_TEXTURE0);
    setInt(s, "cubemap", 0);
    setMat4(s, "viewProjectionSansTranslation", viewProjectionSansTranslation);

    glDepthMask(GL_FALSE);
    glBindVertexArray(cubemapVao());
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->openGlHandle);
            
    glDrawArrays(GL_TRIANGLES, 0, 36);
            
    glDepthMask(GL_TRUE);
}
