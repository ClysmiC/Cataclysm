#include "Gjk.h"
#include <assert.h>

// Reference:
// https://www.youtube.com/watch?v=Qupqu1xe7Io

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

GjkResult gjk(ICollider* a, ICollider* b)
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
            result.penetrationVector = Vec3(0, 0, 0); // TODO: use EPA to calculate this
            break;
        }
    }

    if (iteration >= MAX_ITERATIONS)
    {
        assert(false); // TODO: investigate what a good number of max iterations should be.
    }

    return result;
}
