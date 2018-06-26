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
    void drawSphere(Vec3 position, float32 radius, CameraComponent* camera, Transform* cameraXfm);
    void drawCyllinder(Vec3 position, float32 radius, float32 length, Axis3D axis, CameraComponent* camera, Transform* cameraXfm);
    void drawCapsule(Vec3 position, float32 radius, float32 length, Axis3D axis, CameraComponent* camera, Transform* cameraXfm);
    
    void drawRect3(Vec3 center, Vec3 dimensions, Quaternion orientation, CameraComponent* camera, Transform* cameraXfm);
    void drawAARect3(Vec3 center, Vec3 dimensions, CameraComponent* camera, Transform* cameraXfm);

    void drawAabb(Entity entity, CameraComponent* camera, Transform* cameraXfm);

    // todo
    void drawLine(Vec3 start, Vec3 end, CameraComponent* camera, Transform* cameraXfm);

    void drawCollider(ColliderComponent* collider, CameraComponent* camera, Transform* cameraXfm);

private:
    uint32 vbo;
    uint32 ebo;
    uint32 vao;

    // VBO contains all the vertices for all the primitives, in the following order:
    // - cube
    // - sphere
    // - line
    //
    uint32 cubeVerticesCount;
    uint32 sphereVerticesCount;
    uint32 lineVerticesCount;

    uint32 cubeIndicesCount;
    uint32 sphereIndicesCount;
    uint32 lineIndicesCount;
    
    void _init_cube();
    void _init_sphere();

    DebugDraw() = default;
};

