#pragma once

#include "als/als_math.h"
#include "Aabb.h"
#include <vector>

struct ConvexHullEdge
{
    ConvexHullEdge(uint32 index0, uint32 index1)
        : index0(index0)
        , index1(index1)
    {}
    
    uint32 index0;
    uint32 index1;
};

struct ConvexHull
{
    std::vector<Vec3> positions;
    std::vector<ConvexHullEdge> edges;

    Aabb bounds;
};

Vec3 approximateHullCentroid(ConvexHull* hull);
void recalculatePositionsRelativeToCentroid(ConvexHull* hull, Vec3 centroid);
void recalculateBounds(ConvexHull* hull);
