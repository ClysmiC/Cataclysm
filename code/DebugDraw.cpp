#include "als_math.h"
#include "DebugDraw.h"
#include "GL/glew.h"
#include "ResourceManager.h"

#include "Entity.h"
#include "Ecs.h"

#include "TransformComponent.h"

#define SPHERE_SUBDIVISIONS 3
#define CIRCLE_EDGES       32

DebugDraw&
DebugDraw::instance()
{
    static DebugDraw *instance = new DebugDraw();

    return *instance;
}

void DebugDraw::_init_cube()
{
    float32 cubeVertices[] = {
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

    uint32 verticesCount = cubeVerticesCount + sphereVerticesCount + lineVerticesCount;
    uint32 indicesCount = cubeIndicesCount + sphereIndicesCount + lineIndicesCount;

    glBindVertexArray(vao);
    {
        // Fill in
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeVertices), cubeVertices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(cubeIndices), cubeIndices);
    }
    glBindVertexArray(0);
}

void DebugDraw::_init_sphere()
{
	constexpr int32 totalSphereTriangles = 8 * powi(4, SPHERE_SUBDIVISIONS);

    struct Triangle
    {
        Vec3 a, b, c;
        Triangle() = default;
        Triangle(Vec3 a, Vec3 b, Vec3 c) : a(a), b(b), c(c) {};
    };
	
	Triangle unitSphereTriangles[totalSphereTriangles];

	// Step 1: construct octahedron (should be an equal number of '4's
	// to the number of subdivisions). Could replace array with vector
	// and use dynamic amount of subdivisions.

	// Note: Triangles are stored counter-clockwise

	// top-back-right
	unitSphereTriangles[0].a = Vec3(0, 1, 0);
	unitSphereTriangles[0].b = Vec3(1, 0, 0);
	unitSphereTriangles[0].c = Vec3(0, 0, -1);

	// top-front-right
	unitSphereTriangles[1].a = Vec3(0, 1, 0);
	unitSphereTriangles[1].b = Vec3(0, 0, 1);
	unitSphereTriangles[1].c = Vec3(1, 0, 0);

	// top-front-left
	unitSphereTriangles[2].a = Vec3(0, 1, 0);
	unitSphereTriangles[2].b = Vec3(-1, 0, 0);
	unitSphereTriangles[2].c = Vec3(0, 0, 1);

	// top-back-left
	unitSphereTriangles[3].a = Vec3(0, 1, 0);
	unitSphereTriangles[3].b = Vec3(0, 0, -1);
	unitSphereTriangles[3].c = Vec3(-1, 0, 0);

	// bot-back-right
	unitSphereTriangles[4].a = Vec3(0, -1, 0);
	unitSphereTriangles[4].b = Vec3(0, 0, -1);
	unitSphereTriangles[4].c = Vec3(1, 0, 0);

	// bot-front-right
	unitSphereTriangles[5].a = Vec3(0, -1, 0);
	unitSphereTriangles[5].b = Vec3(1, 0, 0);
	unitSphereTriangles[5].c = Vec3(0, 0, 1);

	// bot-front-left
	unitSphereTriangles[6].a = Vec3(0, -1, 0);
	unitSphereTriangles[6].b = Vec3(0, 0, 1);
	unitSphereTriangles[6].c = Vec3(-1, 0, 0);

	// bot-back-left
	unitSphereTriangles[7].a = Vec3(0, -1, 0);
	unitSphereTriangles[7].b = Vec3(-1, 0, 0);
	unitSphereTriangles[7].c = Vec3(0, 0, -1);

	// Subdivide the existing triangles 5 times
	for(int32 currentSubdivision = 0; currentSubdivision < SPHERE_SUBDIVISIONS; currentSubdivision++)
	{
		int32 trianglesPreDivide = 8 * powi(4, currentSubdivision);
		int32 trianglesPostDivide = trianglesPreDivide * 4;

		int32 newTriangleIndex = trianglesPostDivide - 1;

		// Subdivide from end to beginning and place the new from back
		// to front, so that by the time the "before" triangles start
		// getting overwritten, they wil have already been subdivided.
		for(int32 i = trianglesPreDivide - 1; i >= 0; i--)
		{
			Triangle t = unitSphereTriangles[i];

			Vec3 d = (t.a + t.b) / 2.0f;
			Vec3 e = (t.b + t.c) / 2.0f;
			Vec3 f = (t.a + t.c) / 2.0f;

			Triangle t1 = { t.a, d, f };
			Triangle t2 = { t.b, e, d };
			Triangle t3 = { t.c, f, e };
			Triangle t4 = { d, e, f };

			assert(t1.a.x >= -1 && t1.a.x <= 1);
			assert(t1.a.y >= -1 && t1.a.y <= 1);
			assert(t1.a.z >= -1 && t1.a.z <= 1);

			assert(t2.a.x >= -1 && t2.a.x <= 1);
			assert(t2.a.y >= -1 && t2.a.y <= 1);
			assert(t2.a.z >= -1 && t2.a.z <= 1);

			assert(t3.a.x >= -1 && t3.a.x <= 1);
			assert(t3.a.y >= -1 && t3.a.y <= 1);
			assert(t3.a.z >= -1 && t3.a.z <= 1);

			assert(t4.a.x >= -1 && t4.a.x <= 1);
			assert(t4.a.y >= -1 && t4.a.y <= 1);
			assert(t4.a.z >= -1 && t4.a.z <= 1);

			unitSphereTriangles[newTriangleIndex] = t1;
			newTriangleIndex--;
			unitSphereTriangles[newTriangleIndex] = t2;
			newTriangleIndex--;
			unitSphereTriangles[newTriangleIndex] = t3;
			newTriangleIndex--;
			unitSphereTriangles[newTriangleIndex] = t4;
			newTriangleIndex--;
		}
	}

    
    float32 vertexData[totalSphereTriangles * 9];
    int32 index = 0;
    
    for(int32 i = 0; i < totalSphereTriangles; i++)
	{
		Triangle *t = unitSphereTriangles + i;

        t->a.normalizeInPlace();
        t->b.normalizeInPlace();
        t->c.normalizeInPlace();

        vertexData[index++] = t->a.x;
        vertexData[index++] = t->a.y;
        vertexData[index++] = t->a.z;
        
        vertexData[index++] = t->b.x;
        vertexData[index++] = t->b.y;
        vertexData[index++] = t->b.z;
        
        vertexData[index++] = t->c.x;
        vertexData[index++] = t->c.y;
        vertexData[index++] = t->c.z;
	}

	glBindVertexArray(this->vao);
	{
        // Fill in

        // Note: unitSphereTriangles is an array of Triangles, not Vertices
        glBufferSubData(GL_ARRAY_BUFFER, this->cubeVerticesCount * 3 * sizeof(float32), sizeof(vertexData), vertexData);
        // glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, this->cubeIndicesCount * sizeof(uint32), sizeof(indices), indices);
	}
	glBindVertexArray(0);
}

void
DebugDraw::_init_cylinder()
{
    constexpr int cylinderVertices = CIRCLE_EDGES * 2;
    constexpr int floatsPerVertex = 3;
    float32 cylinderVertexData[cylinderVertices * floatsPerVertex];
    
    uint32 index = 0;
    for (uint32 i = 0; i < CIRCLE_EDGES; i++)
    {
        float32 theta = 360.0f / CIRCLE_EDGES * i;
        
        // Top circle
        cylinderVertexData[index++] = cos(TO_RAD(theta));
        cylinderVertexData[index++] = 0.5;
        cylinderVertexData[index++] = sin(TO_RAD(theta));
    }

    for (uint32 i = CIRCLE_EDGES; i < CIRCLE_EDGES * 2; i++)
    {
        float32 theta = 360.0f / CIRCLE_EDGES * i;
        
        // bottom circle
        cylinderVertexData[index++] = cos(TO_RAD(theta));
        cylinderVertexData[index++] = -0.5;
        cylinderVertexData[index++] = sin(TO_RAD(theta));
    }

    constexpr int circleIndices = CIRCLE_EDGES * 2;   // (both circles, drawn using line-loop)
    constexpr int cylinderVerticalIndices = CIRCLE_EDGES * 2; // drawn using normal line
    uint32 vertexIndices[circleIndices + cylinderVerticalIndices];

    uint32 baseIndex = this->cubeVerticesCount + this->sphereVerticesCount + this->lineVerticesCount;

    for (uint32 i = 0; i < circleIndices / 2; i++)
    {
        // Top circle
        vertexIndices[i] = baseIndex + i;
    }

    for (uint32 i = circleIndices / 2; i < circleIndices; i++)
    {
        // Bottom circle
        vertexIndices[i] = baseIndex + i;
    }

    for (uint32 i = circleIndices; i < circleIndices + cylinderVerticalIndices; i += 2)
    {
        uint32 lineNumber = baseIndex + i - circleIndices;
        
        // Vertical lines
        vertexIndices[i]     = lineNumber; // bottom
        vertexIndices[i + 1] = lineNumber + (circleIndices / 2); // top
    }

    uint32 vboOffset = (this->cubeVerticesCount + this->sphereVerticesCount + this->lineVerticesCount) * 3 * sizeof(float32);
    uint32 eboOffset = (this->cubeIndicesCount + this->sphereIndicesCount + this->lineIndicesCount) * sizeof(uint32);

    glBindVertexArray(vao);
    {
        // Fill in
        glBufferSubData(GL_ARRAY_BUFFER, vboOffset, sizeof(cylinderVertexData), cylinderVertexData);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, eboOffset, sizeof(vertexIndices), vertexIndices);
    }
    glBindVertexArray(0);
}

void
DebugDraw::_init_capsule()
{
}

void
DebugDraw::init()
{
    color = Vec3(0, 1, 0);

    shader = ResourceManager::instance().initShader("shader/debugDraw.vert", "shader/debugDraw.frag", true);

    uint32 cubeLines = 12;
    this->cubeVerticesCount = 8;
    this->cubeIndicesCount = cubeLines * 2;

    // TODO: combine shared vertices and use drawElements instead of drawArrays for all
    //       shapes (if possible.... the math might be too tricky)
    
    uint32 sphereTrianglesCount = 8 * powi(4, SPHERE_SUBDIVISIONS);
    this->sphereVerticesCount = 3 * sphereTrianglesCount;
    this->sphereIndicesCount = 0; // (uses drawArrays)
    
    this->lineVerticesCount = 2;
    this->lineIndicesCount = 0;   // (uses drawArrays)

    uint32 cylinderCircleIndices = CIRCLE_EDGES * 2;   // (both circles, drawn using line-loop)
    uint32 cylinderVerticalIndices = CIRCLE_EDGES * 2; // drawn using normal line
    this->cylinderVerticesCount = CIRCLE_EDGES * 2;
    this->cylinderIndicesCount = cylinderCircleIndices + cylinderVerticalIndices;

    // TODO:
    this->capsuleVerticesCount = 0;
    this->capsuleIndicesCount = 0;

    uint32 verticesCount = cubeVerticesCount + sphereVerticesCount + lineVerticesCount + cylinderVerticesCount + capsuleVerticesCount;
    uint32 indicesCount = cubeIndicesCount + sphereIndicesCount + lineIndicesCount + cylinderIndicesCount + capsuleIndicesCount;
    
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->ebo);
    glGenVertexArrays(1, &this->vao);
    
    glBindVertexArray(this->vao);
    {
        uint32 bufferSize = sizeof(float32) * 3 * verticesCount + 16;
        // Allocate
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            bufferSize,
            0,
            GL_STATIC_DRAW); // TODO: since the line segment vertices can be modified every frame, should they be in their own GL_DYNAMIC_DRAW buffer?

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(uint32) * indicesCount,
            0,
            GL_STATIC_DRAW);

        // VBO is just 3 floats per vertex
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float32), (void*)0);
        glEnableVertexAttribArray(0);

        _init_cube();
        _init_sphere();
        _init_cylinder();
        _init_capsule();

        auto v = glGetError();
        assert(v == GL_NO_ERROR);
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawSphere(Vec3 position, float radius, CameraComponent* camera, Transform* cameraXfm)
{
    // Note: since our VBO has radius 1, 'radius' is synonymous with scale
    Mat4 transform;
    transform.scaleInPlace(Vec3(radius));
    transform.translateInPlace(position);

    Mat4 view = worldToView(cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", camera->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    {

        // TODO: construct sphere in way we can use GL_LINES and ebo instead of drawing triangles in wireframe mode.
        // this would save us from having to toggle wireframe mode and face culling

        glDisable(GL_CULL_FACE);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // enable wireframe
        {
            glDrawArrays(GL_TRIANGLES, this->cubeVerticesCount, this->sphereVerticesCount);
        }
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glEnable(GL_CULL_FACE);
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawRect3(Vec3 center, Vec3 dimensions, Quaternion orientation, CameraComponent* camera, Transform* cameraXfm)
{
    // Note: since our VBO has dimensions 1x1x1, 'dimensions' is synonymous with scale
    Mat4 transform;
    transform.scaleInPlace(dimensions);
    transform.rotateInPlace(orientation);
    transform.translateInPlace(center);

    Mat4 view = worldToView(cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", camera->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    glDrawElements(GL_LINES, this->cubeIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void
DebugDraw::drawRect3Aa(Vec3 center, Vec3 dimensions, CameraComponent* camera, Transform* cameraXfm)
{
    Quaternion identity;
    drawRect3(center, dimensions, identity, camera, cameraXfm);
    
    // Mat4 transform;
    // transform.scaleInPlace(dimensions);
    // transform.translateInPlace(center);

    // Mat4 view = worldToView(cameraXfm);
    
    // bind(shader);
    // setMat4(shader, "model", transform);
    // setMat4(shader, "view", view);
    // setMat4(shader, "projection", camera->projectionMatrix);
    // setVec3(shader, "debugColor", color);

    // glBindVertexArray(this->vao);
    // glDrawElements(GL_LINES, this->cubeIndicesCount, GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0);
}

void
DebugDraw::drawCylinderAa(Vec3 center, float32 radius, float32 length, Axis3D axis, CameraComponent* camera, Transform* cameraXfm)
{
    // TODO: rotate based on axis... default is Y

    Mat4 transform;
    transform.scaleInPlace(Vec3(radius, length, radius));
    // transform.rotateInPlace(orientation);
    transform.translateInPlace(center);

    Mat4 view = worldToView(cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", camera->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    {
        constexpr int circleIndices = CIRCLE_EDGES * 2;   // (both circles, drawn using line-loop)
        constexpr int cylinderVerticalIndices = CIRCLE_EDGES * 2; // drawn using normal line
        
        // Draw top circle
        glDrawElements(GL_LINE_LOOP, circleIndices / 2, GL_UNSIGNED_INT,
                       (void*)(sizeof(uint32) * (this->cubeIndicesCount + this->sphereIndicesCount + this->lineIndicesCount))
                      );

        // // Draw bottom circle
        glDrawElements(GL_LINE_LOOP, circleIndices / 2, GL_UNSIGNED_INT,
                        (void*)(sizeof(uint32) * (this->cubeIndicesCount + this->sphereIndicesCount + this->lineIndicesCount + circleIndices / 2))
                       );

        // // Draw vertical lines
        glDrawElements(GL_LINES, cylinderVerticalIndices, GL_UNSIGNED_INT,
                        (void*)(sizeof(uint32) * (this->cubeIndicesCount + this->sphereIndicesCount + this->lineIndicesCount + circleIndices))
                       );
    }
    glBindVertexArray(0);    
}

void
DebugDraw::drawLine(Vec3 start, Vec3 end, CameraComponent* camera, Transform* cameraXfm)
{
    Mat4 transform;
    Mat4 view = worldToView(cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", camera->projectionMatrix);
    setVec3(shader, "debugColor", color);
    
    glBindVertexArray(this->vao);
	{
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

        // Fill in VBO
        float32 vertexData[] = { start.x, start.y, start.z, end.x, end.y, end.z };
        uint32 prevVerticesCount = this->cubeVerticesCount + this->sphereVerticesCount;
        uint32 offset = prevVerticesCount * 3 * sizeof(float32);
        
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertexData), vertexData);

        // Draw
        glDrawArrays(GL_LINES, prevVerticesCount, 2);
	}
	glBindVertexArray(0);
}

void
DebugDraw::drawCollider(ColliderComponent* collider, CameraComponent* cameraComponent, Transform* cameraXfm)
{
    switch(collider->type)
    {
        case ColliderType::RECT3:
        {
            drawRect3(colliderCenter(collider),
                      Vec3(
                          scaledXLength(collider),
                          scaledYLength(collider),
                          scaledZLength(collider)
                      ),
                      getTransformComponent(collider->entity)->orientation,
                      cameraComponent,
                      cameraXfm);
        } break;

        default:
        {
            // Not yet implemented
            assert(false);
        }
    }
}

void drawCylinderAa(Vec3 center, float32 radius, float32 length, Axis3D axis, CameraComponent* camera, Transform* cameraXfm);
void drawCylinder  (Vec3 center, float32 radius, float32 length, Axis3D axis, Quaternion orientation, CameraComponent* camera, Transform* cameraXfm);

void drawCapsuleAa(Vec3 position, float32 radius, float32 length, Axis3D axis, CameraComponent* camera, Transform* cameraXfm);
void drawCapsule  (Vec3 position, float32 radius, float32 length, Axis3D axis, Quaternion orientation, CameraComponent* camera, Transform* cameraXfm);

void DebugDraw::drawAabb(Entity entity, CameraComponent* camera, Transform* cameraXfm)
{
    TransformComponent* xfm = getTransformComponent(entity);
    RenderComponent* rc = getRenderComponent(entity);
    
    if (rc != nullptr && xfm != nullptr)
    {
        Aabb bounds = transformedAabb(rc->submesh->mesh->bounds, xfm);
        drawRect3Aa(bounds.center, bounds.halfDim * 2, camera, cameraXfm);
    }
}
