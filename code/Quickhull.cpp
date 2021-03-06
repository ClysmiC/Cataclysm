#include "als/als_math.h"
#include "als/als_temp_alloc.h"
#include <vector>
#include "ConvexHull.h"

//
// References: 
// [1] https://archive.org/details/GDC2014Gregorius
// [2] https://www.cs.ubc.ca/~lloyd/java/quickhull3d.html
//

#define DO_MERGE true

struct QHVertex;
struct QHHalfEdge;
struct QHFace;
struct QHHull;

struct QHVertex
{
    int index;

    QHVertex* next = nullptr; // Linked list
};

struct QHHalfEdge
{
    QHHalfEdge* next;
    QHHalfEdge* prev;
    QHHalfEdge* twin;
    QHFace* face;
    QHVertex* start;
    QHVertex* end;
};

struct QHFace
{
    QHFace* next = nullptr;
    QHFace* prev = nullptr;
    
    QHHalfEdge* edge0;

    QHVertex* conflictList = nullptr; // Linked list. Furthest conflict is head, but it is not necessarily sorted, since resolving head will orphan the rest of the list anyways.
    QHVertex* conflictListTail = nullptr; // Storing the tail lets us easily orphan the entire conflict list by just prepending it to tho unclaimed list
    float32 furthestConflictDist = 0;

    Vec3 centroid;
    Vec3 normal;

    // Used for horizon building
    bool visited;
};

struct QHContext
{
    QHHalfEdge* edgePool;
    uint32 edgesGrabbedFromPool = 0;
    QHHalfEdge* reclaimedEdges = nullptr; // Linked list

    QHFace* facePool;
    uint32 facesGrabbedFromPool = 0;
    QHFace* reclaimedFaces = nullptr; // Linked list

    Vec3* pointSoup = nullptr;
    uint32 pointSoupCount = 0;

    // Vertices don't grow/shrink like faces/edges, so we can just keep
    // a straight array
    QHVertex* vertices;

    QHFace* hull;
    QHVertex* unclaimedConflicts = nullptr; // When we remove faces, we orphan their conflicts into this list

    // Used during horizon building phase
    QHHalfEdge* horizon = nullptr;
    QHHalfEdge* horizonTail = nullptr;

    float32 epsilon;
};

void initializeContext(QHContext* context, Vec3* pointSoup, uint32 pointSoupCount)
{
    // Derived from V - E + F = 2
    uint32 maxEdgeCount = 3 * pointSoupCount - 6;
    uint32 maxHalfEdgeCount = maxEdgeCount * 2;
    uint32 maxFaceCount = 2 * pointSoupCount - 4;

    context->pointSoup = pointSoup;
    context->pointSoupCount = pointSoupCount;

    // Allocate more than the max so we have ample room for temporary stuff like the horizon edges
    context->edgePool = (QHHalfEdge*)tempAllocArray(sizeof(QHHalfEdge), 2 * maxHalfEdgeCount);
    context->facePool = (QHFace*)tempAllocArray(sizeof(QHFace), 2 * maxFaceCount);
    
    context->vertices = (QHVertex*)tempAllocArray(sizeof(QHVertex), pointSoupCount);
    for (uint32 i = 0; i < pointSoupCount; i++)
    {
        context->vertices[i].index = i;
    }

    // Compute epsilon. Formula taken from Gregorius's GDC talk
    Vec3 maxAbs;
    for (uint32 i = 0; i < pointSoupCount; i++)
    {
        maxAbs.x = fmax(fabs(context->pointSoup[i].x), maxAbs.x);
        maxAbs.y = fmax(fabs(context->pointSoup[i].y), maxAbs.y);
        maxAbs.z = fmax(fabs(context->pointSoup[i].z), maxAbs.z);
    }

    context->epsilon = 3 * (maxAbs.x + maxAbs.y + maxAbs.z) * FLT_EPSILON;
}

bool assignToFurthestFaceIfConflict(QHContext* context, QHVertex* vertex)
{
    QHFace* face = context->hull;
    QHFace* furthestFace = face;
    float32 furthestDist = 0;
    
    while(face)
    {
        Plane facePlane(face->centroid, face->normal);

        float32 dist = signedDistance(context->pointSoup[vertex->index], facePlane);

        if (dist > furthestDist)
        {
            furthestDist = dist;
            furthestFace = face;
        }
                
        face = face->next;
    }

    if (furthestDist <= 0) return false; // Point was inside hull

    if (furthestDist > furthestFace->furthestConflictDist)
    {
        if (!furthestFace->conflictListTail) furthestFace->conflictListTail = vertex;
                
        vertex->next = furthestFace->conflictList;
        furthestFace->conflictList = vertex;
        furthestFace->furthestConflictDist = furthestDist;
    }
    else
    {
        // insert at end of linked list to maintain that head is always furthest

        assert(furthestFace->conflictList); // otherwise, furthestConflictDist should have been 0 so we shouldn't have gotten here

        furthestFace->conflictListTail->next = vertex;
        furthestFace->conflictListTail = vertex;
        vertex->next = nullptr;
    }

    return true; // Point was a conflict
}

QHVertex* nextConflict(QHFace* hull, QHFace** out_conflictedFace)
{
    QHFace* face = hull;

    do
    {
        if (face->conflictList)
        {
            *out_conflictedFace = face;
            return &face->conflictList[0];
        }

        face = face->next;
    } while (face);

    *out_conflictedFace = nullptr;
    return nullptr;
}

QHHalfEdge* newEdge(QHContext* context)
{
    QHHalfEdge* result;
    if (context->reclaimedEdges != nullptr)
    {
        result = context->reclaimedEdges;
        context->reclaimedEdges = context->reclaimedEdges->next;
    }
    else
    {
        result = context->edgePool + context->edgesGrabbedFromPool;
        context->edgesGrabbedFromPool++;
    }

    memset(result, 0, sizeof(QHHalfEdge));
    return result;
}

void reclaimEdge(QHContext* context, QHHalfEdge* edge)
{
    // NOTE: This DOES NOT update the bookkeeping for any other edges/faces, as some of the linked
    //       edges might survive, and will need to get restitched with newly created edges anyways.

    if (context->reclaimedEdges == nullptr)
    {
        context->reclaimedEdges = edge;
        edge->next = nullptr;
    }
    else
    {
        edge->next = context->reclaimedEdges;
        context->reclaimedEdges->prev = edge; // Not really necessary, since the free-list only looks at the next link
        context->reclaimedEdges = edge;
    }
}

void reclaimFace(QHContext* context, QHFace* face)
{
    // NOTE: This DOES upkeep the bookkeeping for the linked list of faces. This is since the list
    //       is merely an unordered list of faces, unlike the edge linked lists which need to maintain
    //       a logical order.

    if (face == context->hull)
    {
        context->hull = face->next;
    }

    if (face->prev) face->prev->next = face->next;
    if (face->next) face->next->prev = face->prev;

    if (face->conflictListTail)
    {
        face->conflictListTail->next = context->unclaimedConflicts;
        context->unclaimedConflicts = face->conflictList;
    }

    if (context->reclaimedFaces == nullptr)
    {
        context->reclaimedFaces = face;
        face->next = nullptr;
    }
    else
    {
        face->next = context->reclaimedFaces;
        context->reclaimedFaces = face;
    }
}

QHFace* newFace(QHContext* context)
{
    QHFace* result;
    if (context->reclaimedFaces != nullptr)
    {
        result = context->reclaimedFaces;
        context->reclaimedFaces = context->reclaimedFaces->next;
    }
    else
    {
        result = context->facePool + context->facesGrabbedFromPool;
        context->facesGrabbedFromPool++;
    }

    memset(result, 0, sizeof(QHFace));
    return result;
}

bool isDefinitelyConvex(QHContext* context, QHHalfEdge* edge)
{
    QHFace* face = edge->face;
    QHFace* otherFace = edge->twin->face;

    Plane facePlane(face->centroid, face->normal);
    Plane otherFacePlane(otherFace->centroid, otherFace->normal);

    if (signedDistance(face->centroid, otherFacePlane) > -context->epsilon) return false;
    if (signedDistance(otherFace->centroid, facePlane) > -context->epsilon) return false;

    return true;
}

bool isVisible(QHContext* context, QHFace* face, Vec3 vantagePoint)
{
    Vec3 vantageToFace = face->centroid - vantagePoint;
    return dot(face->normal, vantageToFace) < 0;
}

void visitFace(QHContext* context, QHFace* face, QHHalfEdge* crossedEdge, Vec3 vantagePoint)
{
    assert(!crossedEdge || face == crossedEdge->face);
    
    face->visited = true;

    assert(isVisible(context, face, vantagePoint));

    // To get a well-ordered list of horizon edges, start the edge CCW w/ respect to the crossed edge
    QHHalfEdge* edge = crossedEdge ? crossedEdge->next : face->edge0;
    QHHalfEdge* startingEdge = edge;

    do
    {
        QHFace* neighbor = edge->twin->face;
        QHHalfEdge* nextEdge = edge->next; // grab on to this early, since we might delete edge during this step, but we still need to iterate to the next one!

        if (!neighbor->visited)
        {
            if (isVisible(context, neighbor, vantagePoint))
            {
                visitFace(context, neighbor, edge->twin, vantagePoint);
                reclaimEdge(context, edge);
            }
            else
            {
                // Add to horizon!
                if (context->horizon == nullptr)
                {
                    edge->next = nullptr;
                    edge->prev = nullptr;
                    context->horizon = edge;
                    context->horizonTail = edge;
                }
                else
                {
                    // Since we are visiting in CCW order, adding to the head would give us CW order, so we add to the tail
                    edge->prev = context->horizonTail;
                    edge->next = nullptr;
                    context->horizonTail->next = edge;
                    context->horizonTail = edge;
                }
            }
        }

        edge = nextEdge;
    } while (edge != startingEdge);

    // Delete face
    reclaimFace(context, face);
}

void releaseEdge(QHContext* context, QHHalfEdge* edge)
{
    if (context->reclaimedEdges)
    {
        edge->next = context->reclaimedEdges;
    }
    else
    {
        edge->next = nullptr;
    }
        
    context->reclaimedEdges = edge;
}

void releaseFace(QHContext* context, QHFace* face)
{
    if (context->reclaimedFaces)
    {
        face->next = context->reclaimedFaces;
    }
    else
    {
        face->next = nullptr;
    }
        
    context->reclaimedFaces = face;
}

Vec3 faceCentroid(QHContext* context, QHFace* face)
{
    Vec3 centroid;
    int denominator = 0;

    QHHalfEdge* edge = face->edge0;
    do
    {
        centroid += context->pointSoup[edge->start->index];
        denominator++;
        edge = edge->next;
    } while (edge != face->edge0);

    centroid /= denominator;

    return centroid;
}

void initializeFace(QHContext* context, QHFace* face, QHVertex* a, QHVertex* b, QHVertex* c)
{
    QHHalfEdge* edge0 = newEdge(context);
    QHHalfEdge* edge1 = newEdge(context);
    QHHalfEdge* edge2 = newEdge(context);

    Plane p(context->pointSoup[a->index], context->pointSoup[b->index], context->pointSoup[c->index]);
    face->normal = p.normal;

    edge0->start = a;
    edge0->end = b;
    edge0->face = face;
    edge0->next = edge1;
    edge0->prev = edge2;
    
    edge1->start = b;
    edge1->end = c;
    edge1->face = face;
    edge1->next = edge2;
    edge1->prev = edge0;

    edge2->start = c;
    edge2->end = a;
    edge2->face = face;
    edge2->next = edge0;
    edge2->prev = edge1;

    face->edge0 = edge0;

    face->centroid = faceCentroid(context, face);
}

void initializeFace(QHContext* context, QHFace* face, QHHalfEdge* horizonEdge, QHVertex* conflict)
{
    QHHalfEdge* horizonToConflict = newEdge(context);
    QHHalfEdge* conflictToHorizon = newEdge(context);

    horizonToConflict->start = horizonEdge->end;
    horizonToConflict->end = conflict;
    horizonToConflict->face = face;
    horizonToConflict->next = conflictToHorizon;
    horizonToConflict->prev = horizonEdge;

    conflictToHorizon->start = conflict;
    conflictToHorizon->end = horizonEdge->start;
    conflictToHorizon->face = face;
    conflictToHorizon->next = horizonEdge;
    conflictToHorizon->prev = horizonToConflict;

    horizonEdge->face = face;
    horizonEdge->next = horizonToConflict;
    horizonEdge->prev = conflictToHorizon;

    face->edge0 = horizonEdge;

    Plane p(context->pointSoup[horizonEdge->start->index], context->pointSoup[horizonEdge->end->index], context->pointSoup[conflict->index]);
    face->normal = p.normal;
    face->centroid = faceCentroid(context, face);
}

void setEdgeTwins(QHHalfEdge* edge, QHHalfEdge* edgeTwin)
{
    assert(edge->start == edgeTwin->end);
    assert(edge->end == edgeTwin->start);

    edge->twin = edgeTwin;
    edgeTwin->twin = edge;
}

// Note: only call this when debugging the implementation!
bool checkInvariants(QHContext* context, QHFace* hull)
{
    QHFace* face = hull;
    int facesChecked = 0;

    while (face)
    {
        int edgesChecked = 0;
        QHHalfEdge* edge = face->edge0;

        do
        {
            // Half-Edge invariants
            if (edge->face  != face)                    return false;
            if (edge        != edge->twin->twin)        return false;
            if (edge->start != edge->twin->end)         return false;
            if (edge->end   != edge->twin->start)       return false;
            if (edge->start != edge->twin->next->start) return false;
            if (edge->end   != edge->twin->prev->end)   return false;
            if (edge->start != edge->prev->end)         return false;
            if (edge->end   != edge->next->start)       return false;
            if (edge        != edge->prev->next)        return false;
            if (edge        != edge->next->prev)        return false;
            if (edge->face  == edge->twin->face)        return false;
            if (edge->start == edge->next->end)         return false;
            if (edge->start == edge->end)               return false;

            // Check that each vertex is touching >= 3 faces
            // (edge is "in" and edge->next is "out" to borrow verbiage from Gregorius's talk)
            if (edge->twin->face == edge->next->twin->face) return false;

            // Convexity invariant
            if (DO_MERGE && !isDefinitelyConvex(context, edge)) return false;

            edge = edge->next;
            edgesChecked++;
        } while (edge != face->edge0);

        // Surely every face must have 3 or more edges!
        if (edgesChecked < 3) return false;

        face = face->next;
        facesChecked++;
    }

    // Surely every hull must have 4 or more faces!
    if (facesChecked < 4) return false;

    return true;
}

void quickHull(Vec3* pointSoup, uint32 pointSoupCount, ConvexHull* hull_out, bool shouldDeduplicate)
{
    hull_out->positions.clear();
    hull_out->edges.clear();

    pushTempAllocFrame();

    // Remove duplicate points @Slow (TODO speed this part up)
    if (shouldDeduplicate)
    {
        for (uint32 i = 0; i < pointSoupCount; i++)
        {
            for (uint32 j = i + 1; j < pointSoupCount; j++)
            {
                if (distanceSquared(pointSoup[i], pointSoup[j]) < 0.0001)
                {
                    // unordered removal of point j
                    pointSoup[j] = pointSoup[pointSoupCount - 1];
                    pointSoupCount--;
                    j--;
                }
            }
        }
    }

    if (pointSoupCount < 4)
    {
        popTempAllocFrame();
        return;
    }

    QHContext context;
    initializeContext(&context, pointSoup, pointSoupCount);

    uint32 aIndex = ~0U, bIndex = ~0U, cIndex = ~0U, dIndex = ~0U;

    //
    // Build initial tetrahedron
    //
    {
        constexpr int extremePointCount = 12;

        // Calculate 2 most extremes in each axis (if you calculate only 1, you are not guaranteed to have 4 unique points)
        //                 minX    minX,  maxX   maxX   minY   minY   maxY   maxY   minZ   minZ   maxZ   maxZ
        int extremes[extremePointCount] = { 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0 };

        for (uint32 i = 1; i < pointSoupCount; i++)
        {
            if (pointSoup[i].x < pointSoup[extremes[0]].x) { extremes[1] = extremes[0];  extremes[0] = i; } else if (pointSoup[i].x < pointSoup[extremes[1]].x ) { extremes[1] = i; }
            if (pointSoup[i].x > pointSoup[extremes[2]].x) { extremes[3] = extremes[2];  extremes[2] = i; } else if (pointSoup[i].x > pointSoup[extremes[3]].x ) { extremes[3] = i; }
            if (pointSoup[i].y < pointSoup[extremes[4]].y) { extremes[5] = extremes[4];  extremes[4] = i; } else if (pointSoup[i].y < pointSoup[extremes[5]].y ) { extremes[5] = i; }
            if (pointSoup[i].y > pointSoup[extremes[6]].y) { extremes[7] = extremes[6];  extremes[6] = i; } else if (pointSoup[i].y > pointSoup[extremes[7]].y ) { extremes[7] = i; }
            if (pointSoup[i].z < pointSoup[extremes[8]].z) { extremes[9] = extremes[8];  extremes[8] = i; } else if (pointSoup[i].z < pointSoup[extremes[9]].z ) { extremes[9] = i; }
            if (pointSoup[i].z > pointSoup[extremes[10]].z) { extremes[11] = extremes[10];  extremes[10] = i; } else if (pointSoup[i].z > pointSoup[extremes[11]].z ) { extremes[11] = i; }
        }

#if 1
        // Debug: check that there are at least 4 unique points in the "extremes"
        int uniquePoints = 1;
        for (int i = 1; i < extremePointCount; i++)
        {
            bool unique = true;
            for (int j = 0; j < i; j++)
            {
                if (extremes[j] == extremes[i])
                {
                    unique = false;
                    break;
                }
            }

            if (unique) uniquePoints++;
        }

        assert(uniquePoints >= 4);
#endif

        // Calculate two extremes that create the longest segment
        {
            float32 maxDistSquared = 0;
            for (uint32 i = 0; i < extremePointCount; i++)
            {
                for (uint32 j = i + 1; j < extremePointCount; j++)
                {
                    if (extremes[i] == extremes[j]) continue;

                    float32 distSquared = distanceSquared(pointSoup[extremes[i]], pointSoup[extremes[j]]);
                    if (distSquared > maxDistSquared)
                    {
                        maxDistSquared = distSquared;
                        aIndex = extremes[i];
                        bIndex = extremes[j];
                    }
                }
            }
        }

        // Calculate which remaining extreme creates a large triangle
        {
            float32 maxDistSquared = 0;
            for (uint32 i = 0; i < extremePointCount; i++)
            {
                uint32 extremeIndex = extremes[i];
                if (extremeIndex == aIndex || extremeIndex == bIndex) continue;

                Vec3 ab = pointSoup[bIndex] - pointSoup[aIndex];
                Vec3 aToExtreme = pointSoup[extremeIndex] - pointSoup[aIndex];
                Vec3 projectionOntoAB = project(aToExtreme, ab);
                Vec3 rejection = aToExtreme - projectionOntoAB;

                float32 distSquared = lengthSquared(rejection);
                if (distSquared > maxDistSquared)
                {
                    maxDistSquared = distSquared;
                    cIndex = extremeIndex;
                }
            }
        }

        // Calculate which remaining extreme creates a large tetrahedron
        {
            float32 maxDist = 0;
            Plane abc = Plane(pointSoup[aIndex], pointSoup[bIndex], pointSoup[cIndex]);

            for (uint32 i = 0; i < extremePointCount; i++)
            {
                uint32 extremeIndex = extremes[i];
                if (extremeIndex == aIndex || extremeIndex == bIndex || extremeIndex == cIndex) continue;

                float32 dist = distance(pointSoup[extremeIndex], abc);
                if (dist > maxDist)
                {
                    maxDist = dist;
                    dIndex = extremeIndex;
                }
            }
            
            assert(aIndex != bIndex);
            assert(aIndex != cIndex);
            assert(aIndex != dIndex);
            assert(bIndex != cIndex);
            assert(bIndex != dIndex);
            assert(cIndex != dIndex);

            // Build the actual tetrahedron
            {
                QHVertex* a = &context.vertices[aIndex];
                QHVertex* b = &context.vertices[bIndex];;
                QHVertex* c = &context.vertices[cIndex];
                QHVertex* d = &context.vertices[dIndex];

                QHFace* face0 = newFace(&context);
                QHFace* face1 = newFace(&context);
                QHFace* face2 = newFace(&context);
                QHFace* face3 = newFace(&context);

                // Potentially re-wind face 0 if D was found in the same direction of its CCW normal
                Vec3 abcToD = pointSoup[dIndex] - abc.point;
                bool rewindFaces = dot(abcToD, abc.normal) > 0;
                initializeFace(&context, face0, a, rewindFaces ? c : b, rewindFaces ? b : c);
                initializeFace(&context, face1, a, rewindFaces ? d : c, rewindFaces ? c : d);
                initializeFace(&context, face2, a, rewindFaces ? b : d, rewindFaces ? d : b);
                initializeFace(&context, face3, b, rewindFaces ? c : d, rewindFaces ? d : c);
            
                if (!rewindFaces)
                {
                    setEdgeTwins(face0->edge0, face2->edge0->next->next);
                    setEdgeTwins(face0->edge0->next, face3->edge0->next->next);
                    setEdgeTwins(face0->edge0->next->next, face1->edge0);
                    setEdgeTwins(face2->edge0, face1->edge0->next->next);
                    setEdgeTwins(face3->edge0, face2->edge0->next);
                    setEdgeTwins(face1->edge0->next, face3->edge0->next);
                }
                else
                {
                    setEdgeTwins(face0->edge0, face1->edge0->next->next);
                    setEdgeTwins(face0->edge0->next, face3->edge0);
                    setEdgeTwins(face0->edge0->next->next, face2->edge0);
                    setEdgeTwins(face1->edge0, face2->edge0->next->next);
                    setEdgeTwins(face2->edge0->next, face3->edge0->next->next);
                    setEdgeTwins(face3->edge0->next, face1->edge0->next);
                }

                face0->next = face1;
                face1->next = face2;
                face2->next = face3;
                face3->next = nullptr;
                
                face0->prev = nullptr;
                face1->prev = face0;
                face2->prev = face1;
                face3->prev = face2;

                context.hull = face0;
            }
        }

        //
        // Assign all conflicted vertices to their furthest-away face
        //
        for (uint32 i = 0; i < pointSoupCount; i++)
        {
            if (i == aIndex || i == bIndex || i == cIndex || i == dIndex) continue;

            assignToFurthestFaceIfConflict(&context, &context.vertices[i]);
        }
    }

    assert(checkInvariants(&context, context.hull));

    //
    // Resolve conflicts
    //
    QHVertex* conflict;
    QHFace* conflictFace;
    while ((conflict = nextConflict(context.hull, &conflictFace)) != nullptr)
    {
        context.horizon = nullptr;
        context.horizonTail = nullptr;

        // Remove conflict from face so that when its conflicts get orphaned, we don't accidentally
        // add this one back to be resolved
        conflictFace->conflictList = conflict->next;
        if (conflictFace->conflictList == nullptr) conflictFace->conflictListTail = nullptr;

        //
        // Build horizon
        //
        {
            // Set all faces to unvisited
            QHFace* faceToSetUnvisited = context.hull;
            while(faceToSetUnvisited)
            {
                faceToSetUnvisited->visited = false;
                faceToSetUnvisited = faceToSetUnvisited->next;
            }

            // Visit faces recursively and build the horizon in the context
            visitFace(&context, conflictFace, nullptr, pointSoup[conflict->index]);
        }

        // When we merge faces, we will do it by iterating over all of the edges of the new
        // faces that we have added to the geometry. Since we add those to the front of the linked list
        // we want to store which face in the linked list is the first one that ISN'T new geometry
        // so we know when to end merging
        QHFace* endMerging = context.hull;

        //
        // Add geometry
        //
        {
            QHHalfEdge* horizonEdge = context.horizon;
            QHHalfEdge* firstConflictToHorizon = nullptr;
            QHHalfEdge* prevHorizonToConflict = nullptr;
            QHHalfEdge* lastHorizonEdge = nullptr;

            // Note: Horizon linked list isn't circular since we never bother to connect the head and tail
            while (horizonEdge)
            {
                //
                //     |--- h ---|      h = horizon
                //     /____ /____      x = current face (other face is prev face)
                //     \    |\   /|     c = conflict
                //      \ x |<--/---\
                //   h2c \  |  /    |---- c2h (twin of prev h2c)
                //       _\\| /
                //          C
                //

                // Grab this here before we mess with the horizonEdge's next pointer in initializeFace(..)
                // (this would be a problem because that pointer serves double duty. In actual mesh, it points
                // to the next half-edge... in horizon list, it behaves more just like a linked list and isn't
                // pointing to a half-edge on the same logical face anymore)
                QHHalfEdge *nextHorizonEdge = horizonEdge->next;

                QHFace* faceToBuild = newFace(&context);
                initializeFace(&context, faceToBuild, horizonEdge, conflict);

                QHHalfEdge* horizonToConflict = horizonEdge->next;
                QHHalfEdge* conflictToHorizon = horizonEdge->prev;
                
                if (!firstConflictToHorizon) firstConflictToHorizon = conflictToHorizon;
                if (prevHorizonToConflict) setEdgeTwins(conflictToHorizon, prevHorizonToConflict);

                prevHorizonToConflict = horizonToConflict;
                lastHorizonEdge = horizonEdge;
                
                horizonEdge = nextHorizonEdge;

                faceToBuild->next = context.hull;
                context.hull->prev = faceToBuild;
                context.hull = faceToBuild;
            }

            setEdgeTwins(lastHorizonEdge->next, firstConflictToHorizon);
        }
        
        //
        // Merge unstable faces
        //
        if (DO_MERGE) {
            QHFace* face = context.hull;
            int debugIterationNum = -1;

            do // Iterate faces
            {
                debugIterationNum++;
                QHHalfEdge* edge = face->edge0;

                // edge0 may change during traversal if it gets merged. We only want to stop traversing
                // once we finish an entire traversal of the edges that started with an unmerged edge0.
                // This guarantees that we actually visit all edges (and all newly absorbed edges)
                bool edge0Merged = true; 

                do // Iterate edges within face
                {

                    if (!isDefinitelyConvex(&context, edge))
                    {
                        QHFace* neighbor = edge->twin->face;
                        if (neighbor == endMerging) endMerging = endMerging->next; // "End merging" is getting merged/reclaimed, so set it to the next one in the list

                        if (face->edge0 == edge)
                        {
                            face->edge0 = edge->next;
                        }

                        // Re-link surrounding edges
                        QHHalfEdge* newNext = edge->twin->next;
                        QHHalfEdge* newPrev = edge->twin->prev;

                        edge->prev->next = newNext;
                        newNext->prev = edge->prev;

                        edge->next->prev = newPrev;
                        newPrev->next = edge->next;

                        // Reclaim the merged edges
                        reclaimEdge(&context, edge->twin);
                        reclaimEdge(&context, edge);

                        // Edges acquired from the neighbor have to have their face reset
                        QHHalfEdge* absorbedEdge = newNext;
                        while (true)
                        {
                            absorbedEdge->face = face;
                            if (absorbedEdge == newPrev) break;

                            absorbedEdge = absorbedEdge->next;
                        }

                        // Reclaim the absorbed face
                        reclaimFace(&context, neighbor);

                        //
                        // Fix topological problems caused by merge.
                        // Namely, ensure every vertex in the face has >= 3 neighboring faces
                        // Omitting this step would lead to issues when the merge looks as follows:
                        //
                        // Case 1:
                        //   /|\             / \            / \
                        //  / | \   merge   /   \   fix    /   \
                        // (--x  )   =>   a(--x  )   =>  a(     )
                        //  \ | /           \ | /          \   /
                        //   \|/             \|/            \ /
                        //                    b              b
                        // Strategy 1: merge the faces and remove the "interior" edges (ax, xb, bx, xa). Replace them by the single "exterior" edge (ab)
                        // Note: We use this strategy when the "other" face is a triangle, since it only has 1 exterior edge that is simple to drop in w/o iteration
                        //
                        //         OR
                        //
                        // Case 2:           a              a
                        //   /|\            /|\            /|\
                        //  / | \   merge  / | \   fix    / | \
                        // (--x  )   =>   (  x  )c  =>   (  |  )c
                        //  \ | /          \ | /          \ | /
                        //   \|/            \|/            \|/
                        //                   b              b
                        // Strategy 2: combine edges ax and xb into ab and combine bx and xa into ba
                        // Note: We use this strategy when the "other" face has >3 edges, since replacing the 2 edges w/ 1 won't lead to a degenerate triangle like it would in the triangle case
                        //
                        // Note: We could always use a more complex version of Strategy 1 where we replace them by a chain of exterior edges (in Case 2, these would be bc and ca), but this would require yet another iteration of edges.

                        // edge0 may change during traversal if it is violating. We only want to stop traversing
                        // once we finish an entire traversal of the edges that started with an unviolating edge0.
                        // This guarantees that we actually visit all edges (and all newly absorbed edges)
                        bool edge0Violating = true;
                        QHHalfEdge* inEdge = face->edge0;
                        do
                        {
                            QHHalfEdge* outEdge = inEdge->next;
                            QHHalfEdge* nextInEdge = outEdge;

                            QHFace* inNeighbor = inEdge->twin->face;
                            QHFace* outNeighbor = outEdge->twin->face;

                            if (inNeighbor == outNeighbor)
                            {
                                if (inEdge == face->edge0) face->edge0 = inEdge->prev;
                                if (outEdge == face->edge0) face->edge0 = outEdge->next;

                                if (inNeighbor->edge0 == inNeighbor->edge0->next->next->next)
                                {
                                    // Neighbor is a triangle. Case/Strategy 1!
                                    QHHalfEdge* exteriorEdge = inEdge->twin->next;
                                    exteriorEdge->face = face;

                                    inEdge->prev->next = exteriorEdge;
                                    exteriorEdge->prev = inEdge->prev;

                                    outEdge->next->prev = exteriorEdge;
                                    exteriorEdge->next = outEdge->next;

                                    if (newNext == outEdge || newNext == inEdge) newNext = outEdge->next;

                                    reclaimEdge(&context, inEdge);
                                    reclaimEdge(&context, outEdge);
                                    reclaimEdge(&context, inEdge->twin);
                                    reclaimEdge(&context, outEdge->twin);
                                    reclaimFace(&context, inNeighbor);

                                    nextInEdge = exteriorEdge;
                                }
                                else
                                {
                                    // Neighbor has >3 edges. Case/Strategy 2!
                                    inEdge->end = outEdge->end;
                                    inEdge->next = outEdge->next;
                                    inEdge->next->prev = inEdge;

                                    // This is the edge that is going towards the vertex
                                    // on the neighbor face
                                    QHHalfEdge* twinInEdge = inEdge->twin->prev;
                                    QHHalfEdge* twinOutEdge = twinInEdge->next;

                                    twinInEdge->end = twinOutEdge->end;
                                    twinInEdge->next = twinOutEdge->next;
                                    twinInEdge->next->prev = twinInEdge;

                                    reclaimEdge(&context, outEdge);
                                    reclaimEdge(&context, twinOutEdge);

                                    setEdgeTwins(inEdge, twinInEdge);

                                    if (newNext == outEdge || newNext == inEdge) newNext = inEdge;

                                    // Since inEdge was modified, we have to check it again next iteration
                                    nextInEdge = inEdge; 
                                }
                            }
                            else
                            {
                                if (inEdge == face->edge0) edge0Violating = false;
                            }

                            inEdge = nextInEdge;
                        } while (edge0Violating || inEdge != face->edge0);

                        edge = newNext;

                        // Update centroid
                        face->centroid = faceCentroid(&context, face);

                        // TODO: update normal? It should be *mostly* the same, otherwise we wouldn't have merged...
                        // Should we also update the positions to make sure that the face is actually coplanar?
                    }
                    else
                    {
                        if (face->edge0 == edge)
                        {
                            edge0Merged = false;
                        }

                        edge = edge->next;
                    }
                } while (edge0Merged || edge != face->edge0);

                face = face->next;
            } while (face != endMerging);
        }

        //
        // Re-assign any unclaimed conflict vertices
        //
        QHVertex* unclaimedVertex = context.unclaimedConflicts;
        while (unclaimedVertex)
        {
            // Grab this before it gets modified in assignToFurthestFaceIfConflict(..)
            QHVertex* nextUnclaimedVertex = unclaimedVertex->next;
            assignToFurthestFaceIfConflict(&context, unclaimedVertex);
            
            unclaimedVertex = nextUnclaimedVertex;
        }
        
        context.unclaimedConflicts = nullptr;

        assert(checkInvariants(&context, context.hull));
    }

    //
    // Build output
    //
    {
        // Maps vertex index from it's point soup index to it's hull index
        uint32* hullIndices = (uint32*)tempAllocArray(sizeof(uint32), pointSoupCount);
        memset(hullIndices, 0xFF, sizeof(uint32) * pointSoupCount);

        QHFace* face = context.hull;
        while (face)
        {
            QHHalfEdge* edge = face->edge0;

            do
            {
                uint32 startSoupIndex = edge->start->index;
                uint32 endSoupIndex = edge->end->index;

                uint32* startHullIndex = &hullIndices[startSoupIndex];
                uint32* endHullIndex = &hullIndices[endSoupIndex];

                if (*startHullIndex == 0xFFFFFFFF)
                {
                    *startHullIndex = hull_out->positions.size(); // map the point soup index to the hull index
                    hull_out->positions.push_back(pointSoup[startSoupIndex]); // add the point in the hull at our newly mapped index
                }
            
                if (*endHullIndex == 0xFFFFFFFF)
                {
                    *endHullIndex = hull_out->positions.size();
                    hull_out->positions.push_back(pointSoup[endSoupIndex]);
                }

                assert(*startHullIndex != *endHullIndex);
            
                if (*startHullIndex > *endHullIndex) // since the QH Hull uses half-edges, only add half of them to our output
                {
                    ConvexHullEdge hullEdge(*startHullIndex, *endHullIndex);
                    hull_out->edges.push_back(hullEdge);
                }

                edge = edge->next;
            } while (edge != face->edge0);
            face = face->next;
        }

        recalculateBounds(hull_out);
    }

    popTempAllocFrame();
}
