#include "Submesh.h"

#include "GL/glew.h"
#include "assert.h"
#include "als_math.h"


Submesh::Submesh(const std::string filename, const std::string submeshName, const std::vector<MeshVertex> &vertices, const std::vector<uint32> &indices, Material* material)
    :
    meshFilename(filename),
    submeshName(submeshName),
    vertices(std::move(vertices)),
    indices(std::move(indices)),
    material(material)
{
    assert(vertices.size() > 0);
    assert(indices.size() % 3 == 0);

    recalculateTangentsAndBitangents(this);

	//
	// Setup OpenGL stuff
	//
	this->openGlInfo.indicesSize = this->indices.size();
	
    glGenVertexArrays(1, &this->openGlInfo.vao);
    glGenBuffers(1, &this->openGlInfo.vbo);
    glGenBuffers(1, &this->openGlInfo.ebo);

    glBindVertexArray(openGlInfo.vao);

    glBindBuffer(GL_ARRAY_BUFFER, this->openGlInfo.vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(MeshVertex), this->vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->openGlInfo.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(uint32), this->indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, texCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, tangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, bitangent));
}

void recalculateTangentsAndBitangents(Submesh* submesh)
{
    struct RunningAverages
    {
        Vec3 tangent;
        Vec3 bitangent;

        int samples;

        void update(Vec3 t, Vec3 b)
        {
            samples++;

            tangent = ((samples - 1) / (float)samples) * tangent + (1 / (float)samples) * t;
            bitangent = ((samples - 1) / (float)samples) * bitangent + (1 / (float)samples) * b;
        }
    };

    std::unordered_map<MeshVertex*, RunningAverages> runningAverages;
    runningAverages.reserve(submesh->vertices.size());

    for (int i = 0; i < submesh->indices.size(); i += 3)
    {
        MeshVertex *v1 = &(submesh->vertices[submesh->indices[i]]);
        MeshVertex *v2 = &(submesh->vertices[submesh->indices[i + 1]]);
        MeshVertex *v3 = &(submesh->vertices[submesh->indices[i + 2]]);

        Vec3 edge12 = v2->position - v1->position;
        Vec3 edge13 = v3->position - v1->position;

        real32 deltaU12 = v2->texCoords.x - v1->texCoords.x;
        real32 deltaV12 = v2->texCoords.y - v1->texCoords.y;

        real32 deltaU13 = v3->texCoords.x - v1->texCoords.x;
        real32 deltaV13 = v3->texCoords.y - v1->texCoords.y;

        // Solve system of equations below for X, Y, and Z
        // to get T and B:
        //
        // edge12 = deltaU12 * T + deltaV12 * B;
        // edge13 = deltaU13 * T + deltaV13 * B;

        // In matrix form:
        //
        // | deltaU12 deltaV12 | | T | = | edge12 |
        // | deltaU13 deltaV13 | | B |   | edge13 |
        //
        //         A               x         b
        // 
        // To solve for x, multiply both sides by inv(A)
        //
        // Inv of 2x2 matrix is  1 / determinate * adjugate
        //
        // Adjugate of A is: |  deltaV13  -deltaV12 |
        //                   | -deltaU13   deltaU12 |
        //
        // Final equation:
        //
        // | T | = 1 / det * |  deltaV13  -deltaV12 | | edge12 | 
        // | B |             | -deltaU13   deltaU12 | | edge13 |

        real32 det = (deltaU12 * deltaV13 - deltaU13 * deltaV12);

        if (det == 0.0f)
        {
            // __debugbreak();

            // This means that the points are collinear in UV space
            // (or that UV's weren't supplied and we assigned them 0,
            // in which case only the normal vector matters)
            continue;
        }

        real32 oneOverDet = 1.0f / det;

        Vec3 t = oneOverDet * ( deltaV13 * edge12 + -deltaV12 * edge13);
        Vec3 b = oneOverDet * (-deltaU13 * edge12 +  deltaU12 * edge13);

        runningAverages[v1].update(t, b);
        runningAverages[v2].update(t, b);
        runningAverages[v3].update(t, b);
    }

    for (MeshVertex& vertex : submesh->vertices)
    {
        RunningAverages avgs = runningAverages[&vertex];

        Vec3 n = vertex.normal;
        Vec3 t = avgs.tangent;
        Vec3 b = avgs.bitangent;

        real32 tripleProduct = dot(cross(n, t), b);
        if (fabs(tripleProduct) < 0.00001)
        {
            // Coplanar averages, just punt
            b = cross(n, t);
        }

        if (avgs.samples == 0)
        {
            // This should only happen if there were 0-determinants or if
            // the UVs were defaulted to 0 (in which case normal mapping just
            // points towards the mesh normal) -- just punt
            t = !equals(n, Vec3(1, 0, 0)) && !equals(n, Vec3(-1, 0, 0)) ? Vec3(1, 0, 0) : Vec3(0, 0, 1);
            b = cross(n, t);
        }

        // Gram-Schmidt to assure ortho-normal and computed handedness (if UV's are mirrored)
        vertex.tangent = t - (dot(t, n) * n);
        vertex.tangent.normalizeInPlace();

        bool rightHanded = dot(cross(n, vertex.tangent), b) > 0;
        vertex.bitangent = cross(vertex.normal, vertex.tangent) * (rightHanded ? 1 : -1);
        vertex.bitangent.normalizeInPlace();
    }

    for (auto vertex : submesh->vertices)
    {
        assert(length(vertex.bitangent) > .99 && length(vertex.bitangent) < 1.01);
        assert(length(vertex.tangent) > .99 && length(vertex.tangent) < 1.01);
        assert(length(vertex.normal) > .99 && length(vertex.normal) < 1.01);
    }
}

