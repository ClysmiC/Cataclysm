#include "als_math.h"
#include "DebugDraw.h"
#include "GL/glew.h"
#include "ResourceManager.h"

#include "Entity.h"
#include "Ecs.h"

#include "TransformComponent.h"

#define SPHERE_SUBDIVISIONS 5

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

    
    float32 vertexFloats[totalSphereTriangles * 9];
    uint32 indices[totalSphereTriangles * 3]; // 0, 1, 2, 3, 4, 5....
    int32 floatIndex = 0;
    int32 intIndex = 0;
    for(int32 i = 0; i < totalSphereTriangles; i++)
	{
		Triangle *t = unitSphereTriangles + i;

        t->a.normalizeInPlace();
        t->b.normalizeInPlace();
        t->c.normalizeInPlace();

        vertexFloats[floatIndex++] = t->a.x;
        vertexFloats[floatIndex++] = t->a.y;
        vertexFloats[floatIndex++] = t->a.z;
        indices[intIndex] = intIndex;
        intIndex++;
        
        vertexFloats[floatIndex++] = t->b.x;
        vertexFloats[floatIndex++] = t->b.y;
        vertexFloats[floatIndex++] = t->b.z;
        indices[intIndex] = intIndex;
        intIndex++;
        
        vertexFloats[floatIndex++] = t->c.x;
        vertexFloats[floatIndex++] = t->c.y;
        vertexFloats[floatIndex++] = t->c.z;
        indices[intIndex] = intIndex;
        intIndex++;
	}

	glBindVertexArray(this->vao);
	{
        // Fill in

        // Note: unitSphereTriangles is an array of Triangles, not Vertices
        glBufferSubData(GL_ARRAY_BUFFER, this->cubeVerticesCount * 3, sizeof(vertexFloats), vertexFloats);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
	}
	glBindVertexArray(0);
}

void
DebugDraw::init()
{
    color = Vec3(0, 1, 0);

    shader = ResourceManager::instance().initShader("shader/debugDraw.vert", "shader/debugDraw.frag", true);

    this->cubeVerticesCount = 8;
    this->sphereVerticesCount = 8 * powi(4, SPHERE_SUBDIVISIONS); // TODO: this has many repeated vertices... we can combine them (the math is hard though!)
    this->lineVerticesCount = 2;

    this->cubeIndicesCount = 24;
    this->sphereIndicesCount = sphereVerticesCount; // TODO: combine shared vertices
    this->lineIndicesCount = 2;

    uint32 verticesCount = cubeVerticesCount + sphereVerticesCount + lineVerticesCount;
    uint32 indicesCount = cubeIndicesCount + sphereIndicesCount + lineIndicesCount;
    
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->ebo);
    glGenVertexArrays(1, &this->vao);
    
    glBindVertexArray(this->vao);
    {
        // Allocate
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float32) * 3 * verticesCount,
            0,
            GL_STATIC_DRAW);

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
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawSphere(Vec3 position, float radius, CameraComponent* camera, Transform* cameraXfm)
{

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
    glDrawElements(GL_LINES, 12 * 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void
DebugDraw::drawAARect3(Vec3 center, Vec3 dimensions, CameraComponent* camera, Transform* cameraXfm)
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

    glBindVertexArray(this->vao);
    glDrawElements(GL_LINES, 12 * 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void
DebugDraw::drawLine(Vec3 start, Vec3 end, CameraComponent* camera, Transform* cameraXfm)
{
    // TODO:
    // set the two line slots in the VBO using world positions start and end
    // use identity matrix as model

    // temporary hack: using existing rect3 debug drawing to draw a line
    Vec3 rectCenter = (start + end) / 2;
    float32 dist = distance(start, end);

    Vec3 dimensions = Vec3(0.1, 0.1, dist);

    Vec3 defaultForward = -Vec3(Axis3D::Z);
    Vec3 rotationAxis = cross(defaultForward, end - start).normalizeInPlace();

    Quaternion rotationNeeded = relativeRotation(defaultForward, end - start);

    drawRect3(rectCenter, dimensions, rotationNeeded, camera, cameraXfm);
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

void DebugDraw::drawAabb(Entity entity, CameraComponent* camera, Transform* cameraXfm)
{
    TransformComponent* xfm = getTransformComponent(entity);
    RenderComponent* rc = getRenderComponent(entity);
    
    if (rc != nullptr && xfm != nullptr)
    {
        Aabb bounds = transformedAabb(rc->submesh->mesh->bounds, xfm);
        drawAARect3(bounds.center, bounds.halfDim * 2, camera, cameraXfm);
    }
}
