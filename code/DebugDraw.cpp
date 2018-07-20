#include "als_math.h"
#include "DebugDraw.h"
#include "GL/glew.h"
#include "ResourceManager.h"

#include "Entity.h"
#include "Ecs.h"

#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "Window.h"

DebugDraw&
DebugDraw::instance()
{
    static DebugDraw *instance = new DebugDraw();
    return *instance;
}

Quaternion DebugDraw::rotateFromYAxisTo(Axis3D axis)
{
    Quaternion result;

    if (axis == Axis3D::Y) return result;

    else if (axis == Axis3D::X)
    {
        return axisAngle(Vec3(0, 0, -1), 90);
    }
    else if (axis == Axis3D::Z)
    {
        return axisAngle(Vec3(1, 0, 0), 90);
    }

    assert(false);
    return result;
}

// @Slow to call this for both sphere and capsule init, but it is a one-time
// cost and avoids heap allocation so I can live with it!
void DebugDraw::_calculate_unit_sphere_vertices(float32* outputArray)
{
    struct Triangle
    {
        Vec3 a, b, c;
        Triangle() = default;
        Triangle(Vec3 a, Vec3 b, Vec3 c) : a(a), b(b), c(c) {};
    };
	
	Triangle unitSphereTriangles[SphereTrianglesCount];

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

    int32 index = 0;
    
    for(int32 i = 0; i < SphereTrianglesCount; i++)
	{
		Triangle *t = unitSphereTriangles + i;

        t->a.normalizeInPlace();
        t->b.normalizeInPlace();
        t->c.normalizeInPlace();

        outputArray[index++] = t->a.x;
        outputArray[index++] = t->a.y;
        outputArray[index++] = t->a.z;
        
        outputArray[index++] = t->b.x;
        outputArray[index++] = t->b.y;
        outputArray[index++] = t->b.z;
        
        outputArray[index++] = t->c.x;
        outputArray[index++] = t->c.y;
        outputArray[index++] = t->c.z;
	}
}

void DebugDraw::_init_cube()
{
    float32 cubeVertices[CubeVerticesCount * 3] = {
        -.5, -.5,  .5,  // front-bot-left
        .5, -.5,  .5,  // front-bot-right
        .5,  .5,  .5,  // front-top-right
        -.5,  .5,  .5,  // front-top-left
        -.5, -.5, -.5,  // back-bot-left
        .5, -.5, -.5,  // back-bot-right
        .5,  .5, -.5,  // back-top-right
        -.5,  .5, -.5,  // back-top-left
    };

    uint32 cubeIndices[CubeIndicesCount] = {
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
    float32 vertexData[SphereVerticesCount * 3];
    _calculate_unit_sphere_vertices(vertexData);

	glBindVertexArray(this->vao);
	{
        // Fill in
        uint32 byteOffset = vertexCountToBytes(CubeVerticesCount);
        glBufferSubData(GL_ARRAY_BUFFER, byteOffset, sizeof(vertexData), vertexData);
	}
	glBindVertexArray(0);
}

void
DebugDraw::_init_cylinder()
{
    float32 cylinderVertexData[CylinderVerticesCount * 3];
    
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

    constexpr int singleCircleIndices = CIRCLE_EDGES; // drawn using line-loop
    constexpr int verticalIndices = CIRCLE_EDGES * 2; // drawn using normal line
    uint32 vertexIndices[singleCircleIndices * 2 + verticalIndices];

    uint32 baseIndex = CubeVerticesCount + SphereVerticesCount + LineVerticesCount;

    for (uint32 i = 0; i < singleCircleIndices; i++)
    {
        // Top circle
        vertexIndices[i] = baseIndex + i;
    }

    for (uint32 i = singleCircleIndices; i < singleCircleIndices * 2; i++)
    {
        // Bottom circle
        vertexIndices[i] = baseIndex + i;
    }

    uint32 lineNumber = 0;
    for (uint32 i = singleCircleIndices * 2; i < singleCircleIndices * 2 + verticalIndices; i += 2)
    {
        vertexIndices[i]     = baseIndex + lineNumber; // bottom
        vertexIndices[i + 1] = baseIndex + lineNumber + singleCircleIndices; // top

        lineNumber++;
    }

    uint32 vboOffset = vertexCountToBytes(CubeVerticesCount + SphereVerticesCount + LineVerticesCount);
    uint32 eboOffset = (CubeIndicesCount + SphereIndicesCount + LineIndicesCount) * sizeof(uint32);

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
    float32 endcapVertexData[CapsuleEndcapTrianglesCount * 3 * 3];
    float32 sphereVertexData[SphereVerticesCount * 3];

    // We don't really know which sphere vertices are on the top or bottom hemisphere,
    // so we test to see if they have y values above or below 0, and then put them in
    // offset them positively or negatively to become capsule endcap triangles
    _calculate_unit_sphere_vertices(sphereVertexData);

    uint32 numEncapVerticesPlaced = 0;
    
    for (uint32 i = 0; i < SphereTrianglesCount; i++)
    {
        uint32 f = 9 * i;

        float32 ax = sphereVertexData[f];
        float32 ay = sphereVertexData[f+1];
        float32 az = sphereVertexData[f+2];

        float32 bx = sphereVertexData[f+3];
        float32 by = sphereVertexData[f+4];
        float32 bz = sphereVertexData[f+5];

        float32 cx = sphereVertexData[f+6];
        float32 cy = sphereVertexData[f+7];
        float32 cz = sphereVertexData[f+8];

        if (maxAbs(maxAbs(ay, by), cy) > 0)
        {
            uint32 index = 9 * numEncapVerticesPlaced++;

            endcapVertexData[index] = ax;
            endcapVertexData[index+1] = ay;
            endcapVertexData[index+2] = az;

            endcapVertexData[index+3] = bx;
            endcapVertexData[index+4] = by;
            endcapVertexData[index+5] = bz;

            endcapVertexData[index+6] = cx;
            endcapVertexData[index+7] = cy;
            endcapVertexData[index+8] = cz;
        }
    }

    float32 verticalVertexData[CIRCLE_EDGES * 2 * 3];

    for (uint32 i = 0; i < CIRCLE_EDGES; i++)
    {
        float32 theta = 360.0f / CIRCLE_EDGES * i;
        
        float32 x = cos(TO_RAD(theta));
        float32 z = sin(TO_RAD(theta));

        uint32 f = 6 * i;
        
        verticalVertexData[f] = x;
        verticalVertexData[f+1] = -0.5;
        verticalVertexData[f+2] = z;

        verticalVertexData[f+3] = x;
        verticalVertexData[f+4] = 0.5;
        verticalVertexData[f+5] = z;
    }

    glBindVertexArray(this->vao);
	{
        // Fill in
        uint32 byteOffset = vertexCountToBytes(CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount);
        glBufferSubData(GL_ARRAY_BUFFER, byteOffset, sizeof(endcapVertexData), endcapVertexData);

        byteOffset += sizeof(endcapVertexData);
        glBufferSubData(GL_ARRAY_BUFFER, byteOffset, sizeof(verticalVertexData), verticalVertexData);
	}
	glBindVertexArray(0);
}

void
DebugDraw::_init_cone()
{
    //
    // Calculate vertices
    //
    float32 coneVertexData[ConeVerticesCount * 3];
    
    {
        // Base circle
        uint32 index = 0;
        for (uint32 i = 0; i < CIRCLE_EDGES; i++)
        {
            float32 theta = 360.0f / CIRCLE_EDGES * i;
        
            coneVertexData[index++] = cos(TO_RAD(theta));
            coneVertexData[index++] = -0.5;
            coneVertexData[index++] = sin(TO_RAD(theta));
        }

        // Top point
        coneVertexData[index++] = 0;
        coneVertexData[index++] = 0.5;
        coneVertexData[index++] = 0;
    }

    //
    // Calculate indices
    //
    uint32 vertexIndices[ConeIndicesCount];
    uint32 baseIndex = CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount + CapsuleVerticesCount;
    uint32 topPointIndex = baseIndex + ConeVerticesCount - 1;

    {
        // Base circle
        for (uint32 i = 0; i < CIRCLE_EDGES; i++)
        {
            vertexIndices[i] = baseIndex + i;
        }

        // Lines from base circle vertices to top
        for (uint32 i = 0; i < CIRCLE_EDGES; i++)
        {
            vertexIndices[CIRCLE_EDGES + i * 2]     = baseIndex + i;
            vertexIndices[CIRCLE_EDGES + i * 2 + 1] = topPointIndex;
        }
    }

    uint32 vboOffset = vertexCountToBytes(CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount + CapsuleVerticesCount);
    uint32 eboOffset = (CubeIndicesCount + SphereIndicesCount + LineIndicesCount + CylinderIndicesCount + CapsuleIndicesCount) * sizeof(uint32);

    glBindVertexArray(vao);
    {
        // Fill in
        glBufferSubData(GL_ARRAY_BUFFER, vboOffset, sizeof(coneVertexData), coneVertexData);

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, eboOffset, sizeof(vertexIndices), vertexIndices);
    }
    glBindVertexArray(0);
}

void DebugDraw::_init_circle()
{
    //
    // Calculate vertices
    //
    float32 circleVertexData[CircleVerticesCount * 3];
    
    // Base circle
    uint32 index = 0;
    for (uint32 i = 0; i < CIRCLE_EDGES; i++)
    {
        float32 theta = 360.0f / CIRCLE_EDGES * i;
        
        circleVertexData[index++] = cos(TO_RAD(theta));
        circleVertexData[index++] = sin(TO_RAD(theta));
        circleVertexData[index++] = -0.5f; // Near plane is at 0.1, so scoot this back a bit. Projection is ortho anyways so no harm
    }

    uint32 vboOffset = vertexCountToBytes(CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount + CapsuleVerticesCount + ConeVerticesCount);

    glBindVertexArray(vao);
    {
        // Fill in
        glBufferSubData(GL_ARRAY_BUFFER, vboOffset, sizeof(circleVertexData), circleVertexData);
    }
    glBindVertexArray(0);
}

void
DebugDraw::init()
{
    color = Vec3(0, 1, 0);

    shader = ResourceManager::instance().initShader("shader/debugDraw.vert", "shader/debugDraw.frag", true);

    uint32 verticesCount = CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount + CapsuleVerticesCount + ConeVerticesCount + CircleVerticesCount;
    uint32 indicesCount = CubeIndicesCount + SphereIndicesCount + LineIndicesCount + CylinderIndicesCount + CapsuleIndicesCount + ConeIndicesCount + CircleIndicesCount;
    
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
        _init_cone();
        _init_circle();

        {
            auto v = glGetError();
            assert(v == GL_NO_ERROR);
        }
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawSphere(Vec3 position, float radius)
{
    // Note: since our VBO has radius 1, 'radius' is synonymous with scale
    Mat4 transform;
    transform.scaleInPlace(Vec3(radius));
    transform.translateInPlace(position);

    Mat4 view = worldToView(this->cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", this->cameraComponent->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    {

        // TODO: construct sphere in way we can use GL_LINES and ebo instead of drawing triangles in wireframe mode.
        // this would save us from having to toggle wireframe mode and face culling

        glDisable(GL_CULL_FACE);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // enable wireframe
        {
            glDrawArrays(GL_TRIANGLES, CubeVerticesCount, SphereVerticesCount);
        }
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glEnable(GL_CULL_FACE);
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawRect3(Vec3 center, Vec3 dimensions, Quaternion orientation)
{
    // Note: since our VBO has dimensions 1x1x1, 'dimensions' is synonymous with scale
    Mat4 transform;
    transform.scaleInPlace(dimensions);
    transform.rotateInPlace(orientation);
    transform.translateInPlace(center);

    Mat4 view = worldToView(this->cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", this->cameraComponent->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    glDrawElements(GL_LINES, CubeIndicesCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void
DebugDraw::drawRect3Aa(Vec3 center, Vec3 dimensions)
{
    Quaternion identity;
    drawRect3(center, dimensions, identity);
}

void
DebugDraw::drawCylinder(Vec3 center, float32 radius, float32 length, Axis3D axis, Quaternion orientation)
{
    Quaternion rotationNeeded = orientation * rotateFromYAxisTo(axis);
    
    Mat4 transform;
    transform.scaleInPlace(Vec3(radius, length, radius));
    transform.rotateInPlace(rotationNeeded);
    transform.translateInPlace(center);

    Mat4 view = worldToView(this->cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", this->cameraComponent->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    {
        uint32 singleCircleIndices = CIRCLE_EDGES;   // drawn using line-loop
        uint32 verticalIndices = CIRCLE_EDGES * 2;   // drawn using normal line
        
        // Draw top circle
        glDrawElements(GL_LINE_LOOP, singleCircleIndices, GL_UNSIGNED_INT,
                       (void*)(sizeof(uint32) * (CubeIndicesCount + SphereIndicesCount + LineIndicesCount))
                      );

        // Draw bottom circle
        glDrawElements(GL_LINE_LOOP, singleCircleIndices, GL_UNSIGNED_INT,
                       (void*)(sizeof(uint32) * (CubeIndicesCount + SphereIndicesCount + LineIndicesCount + singleCircleIndices))
                      );

        // Draw vertical lines
        glDrawElements(GL_LINES, verticalIndices, GL_UNSIGNED_INT,
                       (void*)(sizeof(uint32) * (CubeIndicesCount + SphereIndicesCount + LineIndicesCount + singleCircleIndices * 2))
                      );
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawCylinderAa(Vec3 center, float32 radius, float32 length, Axis3D axis)
{
    Quaternion identity;
    drawCylinder(center, radius, length, axis, identity);
}

void
DebugDraw::drawLine(Vec3 start, Vec3 end)
{
    Mat4 transform;
    Mat4 view = worldToView(this->cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", this->cameraComponent->projectionMatrix);
    setVec3(shader, "debugColor", color);
    
    glBindVertexArray(this->vao);
	{
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

        // Fill in VBO
        float32 vertexData[] = { start.x, start.y, start.z, end.x, end.y, end.z };
        uint32 offset = vertexCountToBytes(CubeVerticesCount + SphereVerticesCount);
        
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertexData), vertexData);

        // Draw
        glDrawArrays(GL_LINES, CubeVerticesCount + SphereVerticesCount, 2);
	}
	glBindVertexArray(0);
}

void
DebugDraw::drawCone(Vec3 position, float32 radius, float32 height, Quaternion orientation)
{
    Mat4 transform;
    transform.scaleInPlace(Vec3(radius, height, radius));
    transform.rotateInPlace(orientation);
    transform.translateInPlace(position);

    Mat4 view = worldToView(this->cameraXfm);
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", this->cameraComponent->projectionMatrix);
    setVec3(shader, "debugColor", this->color);

    glBindVertexArray(this->vao);
    {
        // Draw base circle
        glDrawElements(GL_LINE_LOOP, CIRCLE_EDGES, GL_UNSIGNED_INT,
                       (void*)(sizeof(uint32) * (CubeIndicesCount + SphereIndicesCount + LineIndicesCount + CylinderIndicesCount + CapsuleIndicesCount))
                      );

        // Draw vertical lines
        glDrawElements(GL_LINES, CIRCLE_EDGES * 2, GL_UNSIGNED_INT,
                       (void*)(sizeof(uint32) * (CubeIndicesCount + SphereIndicesCount + LineIndicesCount + CylinderIndicesCount + CapsuleIndicesCount + CIRCLE_EDGES))
                      );
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawArrow(Vec3 start, Vec3 end)
{
    Vec3 startToEnd = end - start;
    Vec3 startToEndNormalized = normalize(startToEnd);

    // Can make this configurable if needed
    float32 coneHeight = .1 * length(startToEnd);
    float32 coneRadius = coneHeight / 2;

    //
    //  ===================|>
    // ^                   ^ ^
    // |                   | |
    // start               | end
    //             cylinder end
    
    Vec3 cylinderEnd = end - (coneHeight * startToEndNormalized);
    float32 cylinderHeight = length(cylinderEnd - start);

    Quaternion orientation = relativeRotation(Vec3(0, 1, 0), startToEndNormalized);

    drawCylinder(
        start + cylinderHeight / 2 * startToEndNormalized,
        coneRadius / 2.5,
        cylinderHeight,
        Axis3D::Y,
        orientation
    );

    drawCone(
        (cylinderEnd + end) / 2,
        coneRadius,
        coneHeight,
        orientation
    );
}

void
DebugDraw::drawCollider(ColliderComponent* collider)
{
    switch(collider->type)
    {
        case ColliderType::RECT3:
        {
            drawRect3(
                colliderCenter(collider),
                Vec3(
                    scaledXLength(collider),
                    scaledYLength(collider),
                    scaledZLength(collider)
                ),
                getTransformComponent(collider->entity)->orientation()
            );
        } break;

        case ColliderType::SPHERE:
        {
            drawSphere(
                colliderCenter(collider),
                scaledRadius(collider)
            );
        } break;

        case ColliderType::CYLINDER:
        {
            drawCylinder(
                colliderCenter(collider),
                scaledRadius(collider),
                scaledLength(collider),
                collider->axis,
                getTransformComponent(collider->entity)->orientation()
            );
        } break;

        case ColliderType::CAPSULE:
        {
            drawCapsule(
                colliderCenter(collider),
                scaledRadius(collider),
                scaledLength(collider),
                collider->axis,
                getTransformComponent(collider->entity)->orientation()
            );
        } break;

        default:
        {
            // Not yet implemented
            assert(false);
        }
    }
}

void
DebugDraw::drawCapsule(Vec3 center, float32 radius, float32 length, Axis3D axis, Quaternion orientation)
{
    Quaternion rotationNeeded = orientation * rotateFromYAxisTo(axis);
    
    Mat4 cylinderXfm;
    cylinderXfm.scaleInPlace(Vec3(radius, length, radius));
    cylinderXfm.rotateInPlace(rotationNeeded);
    cylinderXfm.translateInPlace(center);

    Mat4 topEndcapXfm;
    topEndcapXfm.scaleInPlace(Vec3(radius, radius, radius));
    topEndcapXfm.translateInPlace(0.5 * length * Vec3(Axis3D::Y));
    topEndcapXfm.rotateInPlace(rotationNeeded);
    topEndcapXfm.translateInPlace(center);

    Mat4 botEndcapXfm;
    botEndcapXfm.scaleInPlace(Vec3(radius, radius, radius));
    botEndcapXfm.rotateInPlace(axisAngle(Vec3(Axis3D::X), 180));
    botEndcapXfm.translateInPlace(-0.5 * length * Vec3(Axis3D::Y));
    botEndcapXfm.rotateInPlace(rotationNeeded);
    botEndcapXfm.translateInPlace(center);

    Mat4 view = worldToView(this->cameraXfm);
    
    bind(shader);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", this->cameraComponent->projectionMatrix);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // enable wireframe
        {
            
            // Draw endcaps
            setMat4(shader, "model", topEndcapXfm);
            glDrawArrays(
                GL_TRIANGLES,
                CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount,
                CapsuleEndcapVerticesCount
            );
            
            setMat4(shader, "model", botEndcapXfm);
            glDrawArrays(
                GL_TRIANGLES,
                CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount,
                CapsuleEndcapVerticesCount
            );

            setMat4(shader, "model", cylinderXfm);
            // Draw vertical lines
            glDrawArrays(
                GL_LINES,
                CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount + CapsuleEndcapVerticesCount,
                CIRCLE_EDGES * 2
            );
        }
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glEnable(GL_CULL_FACE);
    }
    glBindVertexArray(0);
}

void DebugDraw::drawCapsuleAa(Vec3 center, float32 radius, float32 length, Axis3D axis)
{
    Quaternion identity;
    drawCapsule(center, radius, length, axis, identity);
}

void DebugDraw::drawAabb(Aabb aabb)
{
    drawRect3Aa(aabb.center, aabb.halfDim * 2);
}

void DebugDraw::drawAabb(Entity entity)
{
    TransformComponent* xfm = getTransformComponent(entity);
    RenderComponent* rc = getRenderComponent(entity);
    
    if (rc != nullptr && xfm != nullptr)
    {
        Aabb bounds = transformedAabb(rc->submesh->mesh->bounds, xfm);
        drawRect3Aa(bounds.center, bounds.halfDim * 2);
    }
}

//
// Drawing 2D
//
void
DebugDraw::drawCirclePixel(Vec2 position, float32 radius)
{
    Mat4 orthoProjection;

    // TODO: don't hard code these
    int w, h;
    glfwGetFramebufferSize(window->glfwWindow, &w, &h);
    float32 aspectRatio = w / (float32)h;
    
    orthoProjection.orthoInPlace(w, aspectRatio, 0.1f, 1.0f);

    Vec2 positionInCameraSpace = position - (Vec2(w, h) / 2);
    
    Mat4 transform;
    transform.scaleInPlace(Vec3(radius, radius, 1));
    transform.translateInPlace(Vec3(positionInCameraSpace.x, positionInCameraSpace.y, 0));

    Mat4 view;
    
    bind(shader);
    setMat4(shader, "model", transform);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", orthoProjection);
    setVec3(shader, "debugColor", color);

    glBindVertexArray(this->vao);
    {
        glDrawArrays(
            GL_LINE_LOOP,
            CubeVerticesCount + SphereVerticesCount + LineVerticesCount + CylinderVerticesCount + CapsuleVerticesCount + ConeVerticesCount,
            CircleVerticesCount
        );
    }
    glBindVertexArray(0);
}

void
DebugDraw::drawCircleViewport(Vec2 position, float32 radius)
{
    Vec2 pixelCoords = viewportCoordinateToPixel(this->window, position);
    float32 scaleFactor = pixelCoords.x;

    drawCirclePixel(pixelCoords, radius * scaleFactor);
}
