#include "Quad.h"
#include "GL/glew.h"

uint32 quadVao()
{
    static uint32 vao;

    if (vao == 0)
    {
        uint32 vbo;

        float32 quadVertices[] = {
            // positions          
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,

            -0.5f, -0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };
        
        glGenBuffers(1, &vbo);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float32), (void*)0);
    }

    return vao;
}

uint32 screenQuadVao()
{
	static uint32 vao;

	if (vao == 0)
	{
		uint32 vbo;

		float32 quadVertices[] = {
			// positions     // uvs       
			-1.0f,  1.0f,    0.0f, 1.0f,
			-1.0f, -1.0f,    0.0f, 0.0f,
			 1.0f,  1.0f,    1.0f, 1.0f,

			-1.0f, -1.0f,    0.0f, 0.0f,
			 1.0f, -1.0f,    1.0f, 0.0f,
			 1.0f,  1.0f,    1.0f, 1.0f
		};

		glGenBuffers(1, &vbo);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float32), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float32), (void*)(2 * sizeof(float32)));
	}

	return vao;
}
