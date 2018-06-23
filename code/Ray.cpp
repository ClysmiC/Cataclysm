#include "Ray.h"
#include "Aabb.h"

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

float32 rayAabbTest(Ray ray, Aabb aabb)
{
    assert(isNormal(ray.direction));

    Vec3 minPoint = aabb.center - aabb.halfDim;
    Vec3 maxPoint = aabb.center + aabb.halfDim;
    
    if (ray.direction.x > 0)
    {
        // test left face
        Plane leftFace = Plane(aabb.center - aabb.halfDim.x * Vec3(Axis3D::X), -Vec3(Axis3D::X));
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
        Plane rightFace = Plane(aabb.center + aabb.halfDim.x * Vec3(Axis3D::X), Vec3(Axis3D::X));
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

    if (ray.direction.y > 0)
    {
        // test bottom face
        Plane bottomFace = Plane(aabb.center - aabb.halfDim.y * Vec3(Axis3D::Y), -Vec3(Axis3D::Y));
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
        Plane topFace = Plane(aabb.center + aabb.halfDim.y * Vec3(Axis3D::Y), Vec3(Axis3D::Y));
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

    if (ray.direction.z > 0)
    {
        // test back face
        Plane backFace = Plane(aabb.center - aabb.halfDim.z * Vec3(Axis3D::Z), -Vec3(Axis3D::Z));
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
        Plane frontFace = Plane(aabb.center + aabb.halfDim.z * Vec3(Axis3D::Z), Vec3(Axis3D::Z));
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

    return -1;
}
