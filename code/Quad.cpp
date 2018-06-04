#include "Quad.h"
#include "GL/glew.h"

uint32 quadVao()
{
	static uint32 vao;

	if (vao == 0)
	{
		uint32 vbo;

		real32 quadVertices[] = {
			// positions          
			-0.5f, -0.5f, 0.0f,
			0.5f,  0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,

			-0.5f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f,
			0.5f,  0.5f, 0.0f
		};
		
		glGenBuffers(1, &vbo);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
	}

	return vao;
}
