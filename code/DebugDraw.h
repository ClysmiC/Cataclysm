#pragma once

#include "Entity.h"
#include "Shader.h"
#include "Transform.h"
#include "ColliderComponent.h"

// not tweakable defines
#define CUBE_VERTICES 8
#define CUBE_LINES    12

// tweakable defines
#define SPHERE_SUBDIVISIONS 3
#define CIRCLE_EDGES       32

struct DebugDraw
{
    //
    // Members
    //
    Shader* shader;
    Vec3 color;

    CameraComponent* cameraComponent;
    TransformComponent* cameraXfm;

    //
    // Singleton
    //
    static DebugDraw& instance();
    
    void init();
    

    //
    // Drawing
    //
    void drawSphere(Vec3 position, float32 radius);
    
    void drawRect3Aa(Vec3 center, Vec3 dimensions);
    void drawRect3  (Vec3 center, Vec3 dimensions, Quaternion orientation);

    void drawCylinderAa(Vec3 center, float32 radius, float32 length, Axis3D axis);
    void drawCylinder  (Vec3 center, float32 radius, float32 length, Axis3D axis, Quaternion orientation);

    void drawCapsuleAa(Vec3 position, float32 radius, float32 length, Axis3D axis);
    void drawCapsule  (Vec3 position, float32 radius, float32 length, Axis3D axis, Quaternion orientation);
    
    void drawAabb(Entity entity);

    void drawLine(Vec3 start, Vec3 end);

    void drawCollider(ColliderComponent* collider);

private:
    uint32 vbo;
    uint32 ebo;
    uint32 vao;

    // VBO contains all the vertices for all the primitives, in the following order:
    // - cube
    // - sphere
    // - line
    // - cylinder
    // - capsule

    inline uint32 vertexCountToBytes(uint32 vertexCount) { return sizeof(float32) * 3 * vertexCount; }
    
    void _init_cube();
    void _init_sphere();
    void _init_cylinder();
    void _init_capsule();

    DebugDraw() = default;

    void _calculate_unit_sphere_vertices(float32* outputArray);
    Quaternion rotateFromYAxisTo(Axis3D axis);
    
    //
    // Cube
    //
    static constexpr uint32 CubeVerticesCount = CUBE_VERTICES;
    static constexpr uint32 CubeIndicesCount = CUBE_LINES * 2;

    //
    // Sphere
    //
    static constexpr uint32 SphereTrianglesCount =
        8 *                               // 8 triangles before subdividing (octahedron)
        powi(4, SPHERE_SUBDIVISIONS);     // Each subdivision multiplies the existing # of triangles by 4

    static constexpr uint32 SphereVerticesCount = 3 * SphereTrianglesCount;
    static constexpr uint32 SphereIndicesCount = 0; // (uses drawArrays)

    //
    // Line
    //
    static constexpr uint32 LineVerticesCount = 2;
    static constexpr uint32 LineIndicesCount = 0;   // (uses drawArrays)

    //
    // Cylinder
    //
    static constexpr uint32 CylinderVerticesCount = CIRCLE_EDGES * 2;
    static constexpr uint32 CylinderIndicesCount =
        CIRCLE_EDGES +      // Top circle, drawn with GL_LINE_LOOP
        CIRCLE_EDGES +      // Bottom circle, drawn with GL_LINE_LOOP
        CIRCLE_EDGES * 2;   // Vertical lines, drawn with GL_LINES

    //
    // Capsule
    //
    static constexpr uint32 CapsuleEndcapTrianglesCount = SphereTrianglesCount / 2;
    static constexpr uint32 CapsuleEndcapVerticesCount = CapsuleEndcapTrianglesCount * 3;
    static constexpr uint32 CapsuleVerticesCount =
        3 * CapsuleEndcapTrianglesCount * 2 + // Half-sphere end-cap on each side
        CIRCLE_EDGES * 2;                     // Lines going vertically. These do NOT connect to vertices on the half-sphere... they are their own vertices
    static constexpr uint32 CapsuleIndicesCount = 0; // (uses drawArrays)
};

