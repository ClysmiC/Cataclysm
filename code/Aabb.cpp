#include "Aabb.h"
#include "float.h"
#include <algorithm>

Aabb transformedAabb(Aabb aabb, Transform* xfm)
{
    //
    // Scale
    //
    Aabb result;
    result.center = hadamard(aabb.center, xfm->scale);
    result.halfDim = hadamard(aabb.halfDim, xfm->scale);
    
    real32 minX = FLT_MAX;
    real32 minY = FLT_MAX;
    real32 minZ = FLT_MAX;

    real32 maxX = -FLT_MAX;
    real32 maxY = -FLT_MAX;
    real32 maxZ = -FLT_MAX;

    //
    // Resize based on rotate
    //
    for (uint32 i = 0; i < 8; i++)
    {
        // Iterate over every combination of ( center +/- halfDim.x, center +/- halfDim.y, center +/- halfDim.z )

        Vec3 corner = result.center +
            ((i & 1 << 0) ? 1 : -1) * result.halfDim.x * Vec3(Axis3D::X) +
            ((i & 1 << 1) ? 1 : -1) * result.halfDim.y * Vec3(Axis3D::Y) +
            ((i & 1 << 2) ? 1 : -1) * result.halfDim.z * Vec3(Axis3D::Z);

        Vec3 rotatedCorner = xfm->orientation * corner;

        minX = std::min(minX, rotatedCorner.x);
        minY = std::min(minY, rotatedCorner.y);
        minZ = std::min(minZ, rotatedCorner.z);
        
        maxX = std::max(maxX, rotatedCorner.x);
        maxY = std::max(maxY, rotatedCorner.y);
        maxZ = std::max(maxZ, rotatedCorner.z);
    }

    Vec3 minPoint = Vec3(minX, minY, minZ);
    Vec3 maxPoint = Vec3(maxX, maxY, maxZ);

    result.halfDim = Vec3(
        (maxPoint.x - minPoint.x) / 2.0f,
        (maxPoint.y - minPoint.y) / 2.0f,
        (maxPoint.z - minPoint.z) / 2.0f
    );
    
    result.center = minPoint + result.halfDim;

    //
    // Translate
    //
    result.center += xfm->position;

    return result;
}

