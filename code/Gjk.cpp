#include "Gjk.h"
#include <assert.h>
#include "float.h"

// Reference:
// https://www.youtube.com/watch?v=Qupqu1xe7Io

#define EPA_MAX_FACES 64

namespace Gjk
{
    struct Simplex
    {
        // Simplex can have up to 4 points, but not necessarily always 4
        // Most "recent" point is always points[count-1]
        Vec3 points [4];
        uint32 pointCount = 0;
    };

    Vec3 minkowskiSupport(ICollider* a, ICollider* b, Vec3 direction)
    {
        Vec3 result = a->support(direction) - b->support(direction);
        return result;
    }

    void addPointToSimplex(Simplex* simplex, Vec3 valueToAdd)
    {
        assert(simplex->pointCount < 4);
        
        simplex->points[simplex->pointCount] = valueToAdd;
        simplex->pointCount++;
    }

    void clearSimplex(Simplex* simplex)
    {
        simplex->pointCount = 0;
    }

    void reconstructSimplex(Simplex* simplex, Vec3* points, uint32 numPoints)
    {
        clearSimplex(simplex);
        for(uint32 i = 0; i < numPoints; i++)
        {
            simplex->points[i] = points[i];
        }

        simplex->pointCount = numPoints;
    }
    
    bool doSimplex(Simplex* simplex, Vec3* direction)
    {
        switch (simplex->pointCount)
        {
            case 2:
            {
                // a = point we just added
                // b = previous point in simplex
                // o = origin

                Vec3 a = simplex->points[1];
                Vec3 b = simplex->points[0];
            
                Vec3 ab = b - a;
                Vec3 ao = -a;

                if(dot(ab, ao) > 0.0f)
                {
                    *direction = cross(cross(ab, ao), ab);
                }
                else
                {
                    *direction = ao;
                }
            } break;

            case 3:
            {
                // a = point we just added
                // b = 2nd newest point in simplex
                // c = first point in simplex
                // o = origin
                Vec3 a = simplex->points[2];
                Vec3 b = simplex->points[1];
                Vec3 c = simplex->points[0];
            
                Vec3 ab = b - a;
                Vec3 ac = c - a;
                Vec3 ao = -a;
                Vec3 triNormal = cross(ab, ac);

                if(dot(cross(triNormal, ac), ao) > 0.0f)
                {
                    if(dot(ac, ao) > 0.0f)
                    {
                        Vec3 newSimplex[2] = { a, c };
                        reconstructSimplex(simplex, newSimplex, 2);
                        *direction = cross(cross(ac, ao), ac);
                    }
                    else
                    {
                        // "Star case" in the reference video
                        if(dot(ab, ao) > 0.0f)
                        {
                            Vec3 newSimplex[2] = { a, b };
                            reconstructSimplex(simplex, newSimplex, 2);
                            *direction = cross(cross(ab, ao), ab);
                        }
                        else
                        {
                            reconstructSimplex(simplex, &a, 1);
                            *direction = ao;
                        }
                    }
                }
                else if(dot(cross(ab, triNormal), ao) > 0.0f)
                {
                    // "Star case" in the reference video
                    if(dot(ab, ao) > 0.0f)
                    {
                        Vec3 newSimplex[2] = { a, b };
                        reconstructSimplex(simplex, newSimplex, 2);
                        *direction = cross(cross(ab, ao), ab);
                    }
                    else
                    {
                        reconstructSimplex(simplex, &a, 1);
                        *direction = ao;
                    }
                }
                else
                {
                    // inside triangle... but which side?
                    if(dot(triNormal, ao) > 0.0f)
                    {
                        // Can we make this better? Instead of searching along the normal, can we search the direction from
                        // the center point of the triangle to the origin? Would this be better? I think so...
                        Vec3 newSimplex[3] = { b, c, a };
                        reconstructSimplex(simplex, newSimplex, 3);
                        
                        *direction = triNormal;
                    }
                    else
                    {
                        // Re-wind triangle (so our tetrahedron simplex can be consistent about knowing which way is CCW)
                        // and search in the opposite direction!
                        *direction = -triNormal;
                    }
                }
                
            } break;

            case 4:
            {
                Vec3 a = simplex->points[3];
                Vec3 b = simplex->points[2];
                Vec3 c = simplex->points[1];
                Vec3 d = simplex->points[0];
            
                Vec3 ab = b - a;
                Vec3 ac = c - a;
                Vec3 ad = d - a;
                Vec3 ao = -a;
            
                Vec3 acbNormal = cross(ac, ab);
                Vec3 adcNormal = cross(ad, ac);
                Vec3 abdNormal = cross(ab, ad);

                if(dot(acbNormal, ao) > 0.0f)
                {
                    Vec3 newSimplex[3] = { a, c, b };
                    reconstructSimplex(simplex, newSimplex, 3);
                    *direction = acbNormal;

                    doSimplex(simplex, direction);
                }
                else if(dot(adcNormal, ao) > 0.0f)
                {
                    Vec3 newSimplex[3] = { a, d, c };
                    reconstructSimplex(simplex, newSimplex, 3);
                    *direction = adcNormal;

                    doSimplex(simplex, direction);
                }
                else if(dot(abdNormal, ao) > 0.0f)
                {
                    Vec3 newSimplex[3] = { a, b, d };
                    reconstructSimplex(simplex, newSimplex, 3);
                    *direction = abdNormal;

                    doSimplex(simplex, direction);
                }
                else
                {
                    // Enclosing the origin!
                    return true;
                }
            } break;

            default:
            {
                assert(false);
            }
        }

        return false;
    }
}

namespace Epa
{
    struct Simplex
    {
        Triangle faces[EPA_MAX_FACES];
        uint32 faceCount;
    };

    void addTriangleToSimplex(Simplex* simplex, Triangle triangle)
    {
        assert(simplex->faceCount < EPA_MAX_FACES);
        simplex->faces[simplex->faceCount] = triangle;
        simplex->faceCount++;
    }
        
    void initSimplexFromGjkSimplex(Simplex* simplex, Gjk::Simplex* gjkSimplex)
    {
        assert(gjkSimplex->pointCount == 4);
            
        Vec3 a = gjkSimplex->points[3];
        Vec3 b = gjkSimplex->points[2];
        Vec3 c = gjkSimplex->points[1];
        Vec3 d = gjkSimplex->points[0];

        simplex->faceCount = 0;
		
        addTriangleToSimplex(simplex, Triangle(a, c, b));
        addTriangleToSimplex(simplex, Triangle(a, d, c));
        addTriangleToSimplex(simplex, Triangle(a, b, d));
        addTriangleToSimplex(simplex, Triangle(b, c, d));
    }

    void unorderedRemoveTriangleFromSimplex(Simplex* simplex, uint32 index)
    {
        assert(index < simplex->faceCount);
        simplex->faces[index] = simplex->faces[simplex->faceCount - 1];
        simplex->faceCount--;
    }

    float32 closestFaceToOrigin(Simplex* simplex, Triangle* out_t, uint32* out_index)
    {
        Vec3 origin;
        assert(simplex->faceCount <= EPA_MAX_FACES);

        // We can "cheat" and project the point onto the plane defined by the triangle
        // because if this gives us a shorter distance than it would be to the triangle
        // then we are guaranteed to have a different face be even closer (due to the
        // simplex's convexity)

        Triangle closestTriangle = simplex->faces[0];
        float32 closestDistance = FLT_MAX;
        uint32 closestIndex = 0;
        
        for(uint32 i = 0; i < simplex->faceCount; i++)
        {
            Triangle triangle = simplex->faces[i];
            Plane triPlane(triangle.a, triangleNormal(triangle));

            float32 dist = distance(
                origin,
                triPlane
            );
            
            if(dist < closestDistance)
            {
                closestDistance = dist;
                closestTriangle = triangle;
                closestIndex = i;
            }
        }

        assert(closestDistance < FLT_MAX);
        *out_t = closestTriangle;
        *out_index = closestIndex;

        return closestDistance;
    }

    void extendSimplex(Simplex* simplex, Vec3 newPoint)
    {
        // @Slow
        // TODO: consider making this simplex a half-edge mesh and re-using the "extend simplex" code from quickhull?
            
        struct Edge
        {
            Vec3 p0, p1;
            Edge() = default;
            Edge(Vec3 p0, Vec3 p1) { this->p0 = p0; this->p1 = p1; }
        };

        Edge edges[EPA_MAX_FACES * 3];
        uint32 edgeCount = 0;

        for(uint32 i = 0; i < simplex->faceCount; i++)
        {
            Triangle t = simplex->faces[i];
            Vec3 normal = triangleNormal(t);

            Vec3 aToNew = newPoint - t.a;
            if(dot(normal, aToNew) > 0)
            {
                // This triangle is "visible", thus must be removed
                
                unorderedRemoveTriangleFromSimplex(simplex, i);
                i--;

                Edge ab(t.a, t.b);
                Edge bc(t.b, t.c);
                Edge ca(t.c, t.a);

                bool abDup = false;
                bool bcDup = false;
                bool caDup = false;
                
                for(uint32 j = 0; j < edgeCount; j++)
                {
                    Edge edge = edges[j];
                    bool removeEdge = false;
                    
                    if(equals(edge.p0, ab.p1) && equals(edge.p1, ab.p0))
                    {
                        abDup = true;
                        removeEdge = true;
                    }

                    if(equals(edge.p0, bc.p1) && equals(edge.p1, bc.p0))
                    {
                        bcDup = true;
                        removeEdge = true;
                    }

                    if(equals(edge.p0, ca.p1) && equals(edge.p1, ca.p0))
                    {
                        caDup = true;
                        removeEdge = true;
                    }
                        
                    // The edge is already in our list, which means it must be removed
                    if(removeEdge)
                    {
                        edges[j] = edges[edgeCount - 1];
                        edgeCount--;
                        j--;
                    }
                }

                if(!abDup)
                {
                    edges[edgeCount] = ab;
                    edgeCount++;
                }

                if(!bcDup)
                {
                    edges[edgeCount] = bc;
                    edgeCount++;
                }

                if(!caDup)
                {
                    edges[edgeCount] = ca;
                    edgeCount++;
                }
            }
        }


        for(uint32 i = 0; i < edgeCount; i++)
        {
            Edge edge = edges[i];
            Triangle newTri = Triangle(edge.p0, edge.p1, newPoint);
            addTriangleToSimplex(simplex, newTri);
        }
    }

    Vec3 epaPenetrationVector(Gjk::Simplex* gjkSimplex, ICollider* a, ICollider* b)
    {
        Simplex simplex;
        initSimplexFromGjkSimplex(&simplex, gjkSimplex);

        Vec3 result;
	
        while(true)
        {
            Triangle face;
            uint32 faceIndex;
            float32 dist = closestFaceToOrigin(&simplex, &face, &faceIndex);
		
            Vec3 faceNormal = triangleNormal(face);
            Vec3 furthestInNormal = Gjk::minkowskiSupport(a, b, faceNormal);
            float32 furthestInNormalDistance = dot(furthestInNormal, faceNormal);

            // SOME tolerance for things like spheres/cylinders which can almost always get a LIIIIITLE closer
            if(furthestInNormalDistance > dist + 0.01)
            {
                extendSimplex(&simplex, furthestInNormal);
            }
            else
            {
                // Simplex can't be expanded, so we have the closest face!
                // Calculate distance to plane
                Plane p(face.a, faceNormal);

                float32 d = distance(Vec3(0), p);
                result = faceNormal * d;

                break;
            }
        }
	
        return result;
    }
}

GjkResult gjk(ICollider* a, ICollider* b, bool calculatePenetrationVector)
{
    using namespace Gjk;

    GjkResult result;

    Simplex simplex;
    
    // Seed the simplex with any point in the Minkowski difference
    Vec3 direction(1, 0, 0);
    Vec3 initialPoint = minkowskiSupport(a, b, direction);

    if (isZeroVector(initialPoint))
    {
        // TODO: handle this edge case for robustness
        assert(false);
        return result;
    }
    
    addPointToSimplex(&simplex, initialPoint);

    // Origin is, by definition, in the opposite direction of the initial point
    direction = -initialPoint;

    const int32 MAX_ITERATIONS = 32;
    int32 iteration = 0;
    for ( ; iteration < MAX_ITERATIONS; iteration++)
    {
        Vec3 pointTowardsOrigin = minkowskiSupport(a, b, direction);

        if (dot(pointTowardsOrigin, direction) < 0.0f)
        {
            break; // No collision
        }

        addPointToSimplex(&simplex, pointTowardsOrigin);

        if (doSimplex(&simplex, &direction))
        {
            result.collides = true;

            if (calculatePenetrationVector)
            {
                result.penetrationVector = Epa::epaPenetrationVector(&simplex, a, b);
            }
            break;
        }
    }

    if (iteration >= MAX_ITERATIONS)
    {
        assert(false); // TODO: investigate what a good number of max iterations should be.
    }

    return result;
}
