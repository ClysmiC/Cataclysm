#include "ConvexHull.h"

Vec3 approximateHullCentroid(ConvexHull* hull)
{
    // Since the hull data structure doesn't store any face information, take the weighted average of the center of
    // the edges, with weight determined by edge length.
    
    Vec3 center;
    float32 totalLength = 0;

    for (ConvexHullEdge& edge: hull->edges)
    {
        Vec3 a = hull->positions[edge.index0];
        Vec3 b = hull->positions[edge.index1];
        Vec3 edgeCenter = (a + b) / 2.0;
        
        float32 length = distance(a, b);
        center += length * edgeCenter;
        totalLength += length;
    }

    center /= totalLength;

    return center;
}

void recalculatePositionsRelativeToCentroid(ConvexHull* hull, Vec3 centroid)
{
    for (Vec3& position: hull->positions)
    {
        position -= centroid;
    }
}
