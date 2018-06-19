#pragma once

#include "Entity.h"
#include "Shader.h"
#include "Transform.h"
#include "ColliderComponent.h"

struct DebugDraw
{
	static DebugDraw& instance();
	
	void init();
	
	Shader* shader;
	Vec3 color;

	// todo
	void drawSphere(Vec3 position, float radius);
	
	void drawRect3(Vec3 center, Vec3 dimensions, Quaternion orientation, CameraComponent* camera, Transform* cameraXfm);
	void drawAARect3(Vec3 center, Vec3 dimensions, CameraComponent* camera, Transform* cameraXfm);

	// todo
	void drawLine(Vec3 start, Vec3 end, CameraComponent* camera, Transform* cameraXfm);

	void drawCollider(ColliderComponent* collider, CameraComponent* camera, Transform* cameraXfm);

private:
	uint32 vbo;
	uint32 ebo;
	uint32 vao;

	DebugDraw() = default;
};

