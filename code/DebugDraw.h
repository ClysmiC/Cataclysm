#pragma once

#include "Entity.h"
#include "Aabb.h"

// not tweakable defines
#define CUBE_VERTICES 8
#define CUBE_LINES    12

// tweakable defines
#define SPHERE_SUBDIVISIONS 3
#define CIRCLE_EDGES       32

struct TransformComponent;
struct ColliderComponent;
struct Window;
struct Shader;

struct DebugDraw
{
    //
    // Members
    //
    Shader* shader;
    Vec3 color;

    CameraComponent* cameraComponent;
    TransformComponent* cameraXfm;

    Window* window; // used for screen to pixel conversions

    //
    // Singleton
    //
    static DebugDraw& instance();
    
    void init();
    

    //
    // Drawing 3D
    //
    void drawSphere(Vec3 position, float32 radius);
    
    void drawRect3Aa(Vec3 center, Vec3 dimensions);
    void drawRect3  (Vec3 center, Vec3 dimensions, Quaternion orientation);

    void drawCylinderAa(Vec3 center, float32 radius, float32 length, Axis3D axis);
    void drawCylinder  (Vec3 center, float32 radius, float32 length, Axis3D axis, Quaternion orientation);

    void drawCapsuleAa(Vec3 position, float32 radius, float32 length, Axis3D axis);
    void drawCapsule  (Vec3 position, float32 radius, float32 length, Axis3D axis, Quaternion orientation);

    void drawCone(Vec3 position, float32 radius, float32 height, Quaternion orientation);

    void drawArrow(Vec3 start, Vec3 end);

    void drawAabb(Aabb aabb);
    void drawAabb(Entity entity);

    void drawLine(Vec3 start, Vec3 end);

    void drawCollider(ColliderComponent* collider); // automatically grabs xfm from entity

    //
    // Drawing 2D
    //
    void drawCirclePixel(Vec2 position, float32 radius);
    void drawCircleViewport(Vec2 position, float32 radius); // 1 radius = width

private:
    uint32 vbo;
    uint32 ebo;
    uint32 vao;

    // VBO contains all the vertices for all the primitives, in the following order:
    //
    //  3D
    //
    // - cube
    // - sphere
    // - line
    // - cylinder
    // - capsule
    // - cone
    //
    // 2D
    //
    // - circle

    inline uint32 vertexCountToBytes(uint32 vertexCount) { return sizeof(float32) * 3 * vertexCount; }

    //
    // 3D
    //
    void _init_cube();
    void _init_sphere();
    void _init_cylinder();
    void _init_capsule();
    void _init_cone();

    //
    // 2D
    //
    void _init_circle();

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

    //
    // Cone
    //
    static constexpr uint32 ConeVerticesCount = CIRCLE_EDGES + 1; // points around the base + 1 point at the top
    static constexpr uint32 ConeIndicesCount =
        CIRCLE_EDGES +     // Base circle, drawn with GL_LINE_LOOP
        CIRCLE_EDGES * 2;  // Line drawn from each of the base circle vertices to the point at the top (GL_LINE)

    //
    // Circle
    //
    static constexpr uint32 CircleVerticesCount = CIRCLE_EDGES;
    static constexpr uint32 CircleIndicesCount = 0; // (uses drawArrays)
};

