#include "Ray.h"
#include "Aabb.h"
#include "Ecs.h"

Ray::Ray(Vec3 position, Vec3 direction)
    : position(position)
    , direction(normalize(direction))
{
}

float32 rayPlaneTest(Ray ray, Plane plane)
{
    assert(isNormal(ray.direction));
    
    // Todo: this doesn't collide with back faces. Should it? Should it be a boolean option?
    if (dot(ray.direction, plane.normal) > 0)
    {
        return -1;
    }

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

float32 rayRect3Test(Ray ray, Vec3 center, Quaternion orientation, Vec3 halfDim)
{
    Vec3 localX = orientation * Vec3(Axis3D::X);
    Vec3 localY = orientation * Vec3(Axis3D::Y);
    Vec3 localZ = orientation * Vec3(Axis3D::Z);

    Vec3 minPoint = center - halfDim;
    Vec3 maxPoint = center + halfDim;
    
    if (dot(ray.direction.x, localX) > 0)
    {
        // test left face
        Plane leftFace = Plane(center - halfDim.x * localX, -localX);
        float32 leftFaceResult = rayPlaneTest(ray, leftFace);
        if (leftFaceResult >= 0)
        {
            Vec3 hitPoint = ray.position + ray.direction * leftFaceResult;
            if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
                hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
            {
                return leftFaceResult;
            }
        }
    }
    else
    {
        // test right face
        Plane rightFace = Plane(center + halfDim.x * localX, localX);
        float32 rightFaceResult = rayPlaneTest(ray, rightFace);
        if (rightFaceResult >= 0)
        {
            Vec3 hitPoint = ray.position + ray.direction * rightFaceResult;
            if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
                hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
            {
                return rightFaceResult;
            }
        }
    }

    if (dot(ray.direction.y, localY) > 0)
    {
        // test bottom face
        Plane bottomFace = Plane(center - halfDim.y * localY, -localY);
        float32 bottomFaceResult = rayPlaneTest(ray, bottomFace);
        if (bottomFaceResult >= 0)
        {
            Vec3 hitPoint = ray.position + ray.direction * bottomFaceResult;
            if (hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x &&
                hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
            {
                return bottomFaceResult;
            }
        }
    }
    else
    {
        // test top face
        Plane topFace = Plane(center + halfDim.y * localY, localY);
        float32 topFaceResult = rayPlaneTest(ray, topFace);
        if (topFaceResult >= 0)
        {
            Vec3 hitPoint = ray.position + ray.direction * topFaceResult;
            if (hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x &&
                hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
            {
                return topFaceResult;
            }
        }
    }

    if (dot(ray.direction.z, localZ) > 0)
    {
        // test back face
        Plane backFace = Plane(center - halfDim.z * localZ, -localZ);
        float32 backFaceResult = rayPlaneTest(ray, backFace);
        if (backFaceResult >= 0)
        {
            Vec3 hitPoint = ray.position + ray.direction * backFaceResult;
            if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
                hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x)
            {
                return backFaceResult;
            }
        }
    }
    else
    {
        // test front face
        Plane frontFace = Plane(center + halfDim.z * localZ, localZ);
        float32 frontFaceResult = rayPlaneTest(ray, frontFace);
        if (frontFaceResult >= 0)
        {
            Vec3 hitPoint = ray.position + ray.direction * frontFaceResult;
            if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
                hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x)
            {
                return frontFaceResult;
            }
        }
    }

    return -1; // todo
}

float32 rayColliderTest(Ray ray, ColliderComponent* collider)
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
            float32 result = rayRect3Test(
                ray,
                colliderCenter(collider), xfm->orientation,
                Vec3(scaledXLength(collider), scaledYLength(collider), scaledZLength(collider) / 2.0f)
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

float32 rayAabbTest(Ray ray, Aabb aabb)
{
    float32 result = rayRect3Test(ray, aabb.center, Quaternion(), aabb.halfDim);
    return result;
}
