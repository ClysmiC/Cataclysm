#include "Gjk.h"

namespace Gjk
{
    struct Simplex
    {
        // Simplex can have up to 4 points, but not necessarily always 4
        // Most "recent" point is always points[count-1]
        Vec3 points [4];
        uint32 pointCount = 0;
    };

    void addPointToSimplex(Simplex* simplex, Vec3 valueToAdd)
    {
        assert(simplex->count < 4);
		
        simplex->points[simplex->count] = valueToAdd;
        simplex->count++;
    }

    void clearSimplex(Simplex* simplex)
    {
        simplex->count = 0;
    }

    // Order doesn't matter when reconstructing simplex, since this is done
    // in gjkDoSimplex(...), and an add(...) call will always occur
    // in gjkCollisionTest(...) before the next gjkDoSimplex(...)
    void reconstructSimplex(Simplex* simplex, Vec3* points, uint32 numPoints)
    {
        simplex->clear();
        for(int i = 0; i < numPoints; i++)
        {
            simplex->points[i] = points[i];
        }

        simplex->count = numPoints;
    }
    
    
}
