#include "Ray.h"
#include "Aabb.h"
#include "Ecs.h"

Ray::Ray(Vec3 position, Vec3 direction)
    : position(position)
    , direction(normalize(direction))
{
}

float32 rayVsPlaneOneSided(Ray ray, Plane plane)
{
    assert(isNormal(ray.direction));

    // Only intersect "front" face
    if (dot(ray.direction, plane.normal) > 0)
    {
        return -1;
    }

    // If plane is behind ray, no intersect
    if (dot(plane.point - ray.position, ray.direction) < 0)
    {
        return -1;
    }

    float32 rayDotNormal = dot(ray.direction, plane.normal);

    if (FLOAT_EQ(rayDotNormal, 0, EPSILON))
    {
        return -1;
    }
    else
    {
        return dot(plane.point - ray.position, plane.normal) / rayDotNormal;
    }
}

float32 rayVsPlaneTwoSided(Ray ray, Plane plane)
{
    assert(isNormal(ray.direction));

    // If plane is behind ray, no intersect
    if (dot(plane.point - ray.position, ray.direction) < 0)
    {
        return -1;
    }

    float32 rayDotNormal = dot(ray.direction, plane.normal);

    if (FLOAT_EQ(rayDotNormal, 0, EPSILON))
    {
        return -1;
    }
    else
    {
        return dot(plane.point - ray.position, plane.normal) / rayDotNormal;
    }
}

float32 rayVsRect3(Ray ray, Vec3 center, Quaternion orientation, Vec3 halfDim)
{    
    Vec3 localX = orientation * Vec3(Axis3D::X);
    Vec3 localY = orientation * Vec3(Axis3D::Y);
    Vec3 localZ = orientation * Vec3(Axis3D::Z);

    // Note: uses 2-sided plane tests, so the sign of the normal doesn't matter
    Plane rightPlane(Vec3(center + localX * halfDim.x),  localX);
    Plane leftPlane (Vec3(center - localX * halfDim.x), -localX);
    
    Plane frontPlane(Vec3(center + localZ * halfDim.z),  localZ);
    Plane backPlane (Vec3(center - localZ * halfDim.z), -localZ);

    Plane topPlane(Vec3(center + localY * halfDim.y),  localY);
    Plane botPlane(Vec3(center - localY * halfDim.y), -localY);

    float32 largestNearT = -1;
    float32 smallestFarT = FLT_MAX;

    struct {
        bool operator() (Ray ray, Plane p1, Plane p2, float32* nearT, float32* farT)
        {
            float32 t1 = rayVsPlaneTwoSided(ray, p1);
            float32 t2 = rayVsPlaneTwoSided(ray, p2);

            if (t1 >= 0 && t2 >= 0)
            {
                // Both intersect
                *nearT = min(t1, t2);
                *farT = max(t1, t2);
            }
            else if (t1 >= 0 || t2 >= 0)
            {
                // Only one intersects
                *nearT = -1;
                *farT = max(t1, t2);
            }
            else
            {
                if (dot(ray.direction, p1.normal) > 0 || dot(ray.direction, p2.normal) > 0)
                {
                    // Non parallel and non-intersecting.
                    // Returning false indicates that the entire rayVsRect3 test fails
                    return false;
                }
                
                *nearT = -1;
                *farT = -1;
            }

            return true;
        }
    } getNearAndFarT;

    float32 nearT, farT;

    uint32 debug_numHits = 0;

    // @Slow: so many if's... can probably cut down on this
    if (!getNearAndFarT(ray, rightPlane, leftPlane, &nearT, &farT)) return -1;
    if (nearT >= 0) largestNearT = max(nearT, largestNearT);
    if (farT >= 0)  smallestFarT = min(farT, smallestFarT);

    if (!getNearAndFarT(ray, topPlane, botPlane, &nearT, &farT)) return -1;
    if (nearT >= 0) largestNearT = max(nearT, largestNearT);
    if (farT >= 0)  smallestFarT = min(farT, smallestFarT);

    if (!getNearAndFarT(ray, frontPlane, backPlane, &nearT, &farT)) return -1;
    if (nearT >= 0) largestNearT = max(nearT, largestNearT);
    if (farT >= 0)  smallestFarT = min(farT, smallestFarT);

    if (largestNearT <= smallestFarT) return largestNearT;
    
    return -1;
}

float32 rayVsCollider(Ray ray, ColliderComponent* collider)
{
    assert(isNormal(ray.direction));
    TransformComponent* xfm = getTransformComponent(collider->entity);
    assert (xfm != nullptr);
    
    Vec3 localX = xfm->right();
    Vec3 localY = xfm->up();
    Vec3 localZ = xfm->back();
    Vec3 center = colliderCenter(collider);

    float32 result = -1;
    
    switch (collider->type)
    {
        case ColliderType::RECT3:
        {
            result = rayVsRect3(
                ray,
                center,
                xfm->orientation,
                Vec3(scaledXLength(collider), scaledYLength(collider), scaledZLength(collider)) / 2.0f
            );
        } break;

        case ColliderType::SPHERE:
        {
            // TODO
        } break;

        case ColliderType::CYLINDER:
        {
            // TODO
        } break;

        case ColliderType::CAPSULE:
        {
            // TODO
        } break;

        default:
        {
            // Not yet implemented
            assert(false);
        }
    }

    return result;
}

float32 rayVsAabb(Ray ray, Aabb aabb)
{
    // @Slow: this uses the general rayVsRect3 test. It could be substantially sped up
    //        since we KNOW we have an AABB, and not any arbitrary rect3
    float32 result = rayVsRect3(ray, aabb.center, Quaternion(), aabb.halfDim);
    return result;
}
