#include "als_math.h"
#include "DebugDraw.h"
#include "GL/glew.h"
#include "ResourceManager.h"

#include "Entity.h"
#include "Ecs.h"

DebugDraw&
DebugDraw::instance()
{
	static DebugDraw *instance = new DebugDraw();

	return *instance;
}

void
DebugDraw::init()
{
	color = Vec3(0, 1, 0);

	shader = ResourceManager::instance().initShader("shader/debugDraw.vert", "shader/debugDraw.frag", true);

	auto v = glGetError();
	
	real32 cubeVertices[] = {
		-.5, -.5,  .5,  // front-bot-left
		 .5, -.5,  .5,  // front-bot-right
		 .5,  .5,  .5,  // front-top-right
		-.5,  .5,  .5,  // front-top-left
		-.5, -.5, -.5,  // back-bot-left
		 .5, -.5, -.5,  // back-bot-right
		 .5,  .5, -.5,  // back-top-right
		-.5,  .5, -.5,  // back-top-left
	};

	uint32 cubeIndices[] = {
		// Front square
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		// Back square
		4, 5,
		5, 6,
		6, 7,
		7, 4,

		// Connecting lines
		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	uint32 cubeVerticesCount = 8;
	uint32 sphereVerticesCount = 1; // TODO
	uint32 lineVerticesCount = 2;

	uint32 cubeIndicesCount = 24;
	uint32 sphereIndicesCount = 1; // TODO
	uint32 lineIndicesCount = 2;

	uint32 verticesCount = cubeVerticesCount + sphereVerticesCount + lineVerticesCount;
	uint32 indicesCount = cubeIndicesCount + sphereIndicesCount + lineIndicesCount;
	
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
	
	glBindVertexArray(vao);
	{
		v = glGetError();

		// Allocate
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(real32) * 3 * verticesCount,
			0,
			GL_STATIC_DRAW);

		v = glGetError();
		// Fill in
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeVertices), cubeVertices);

		v = glGetError();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			sizeof(uint32) * indicesCount,
			0,
			GL_STATIC_DRAW);


		GLint size;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

		v = glGetError();
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(cubeIndices), cubeIndices);

		v = glGetError();
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
		glEnableVertexAttribArray(0);
	}
	glBindVertexArray(0);

	v = glGetError();
}

void
DebugDraw::drawSphere(Vec3 position, float radius)
{
}

void
DebugDraw::drawRect3(Vec3 center, Vec3 dimensions, Quaternion orientation)
{
	// TODO:
}

void
DebugDraw::drawAARect3(Vec3 center, Vec3 dimensions, CameraComponent* camera, TransformComponent* cameraXfm)
{
	// Note: since our VBO has dimensions 1x1x1, 'dimensions' is synonymous with scale
	Mat4 transform;
	transform.scaleInPlace(dimensions);
	transform.translateInPlace(center);

	Mat4 view = worldToView(cameraXfm);
	
	bind(shader);
	setMat4(shader, "model", transform);
	setMat4(shader, "view", view);
	setMat4(shader, "projection", camera->projectionMatrix);
	setVec3(shader, "debugColor", color);

	glBindVertexArray(vao);
	glDrawElements(GL_LINES, 12 * 2, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void
DebugDraw::drawLine(Vec3 start, Vec3 end)
{
	// TODO:
	// set the two line slots in the VBO using world positions start and end
	// use identity matrix as model
}
