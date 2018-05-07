#pragma once

#include "Entity.h"
#include "Shader.h"
#include "Submesh.h"

struct DebugDraw
{
	static DebugDraw& instance();
	void init(Entity camera);
	
	Entity camera;
	
	Shader* shader;
	Vec3 color;
	/* real32 lineThickness = 0.5f; */
	/* bool wireframe = false; */
	
	void drawSphere(Vec3 position, float radius);
	void drawRect3(Vec3 center, Vec3 dimensions, Quaternion orientation);
	void drawAARect3(Vec3 center, Vec3 dimensions);
	void drawLine(Vec3 start, Vec3 end);

private:
	uint32 vbo;
	uint32 ebo;
	uint32 vao;

	Mat4 projection; // TODO: consolidate this with camera

	DebugDraw() = default;
};

