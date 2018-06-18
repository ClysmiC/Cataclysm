#include "Ray.h"
#include "Aabb.h"

Ray::Ray(Vec3 position, Vec3 direction)
	: position(position)
	, direction(normalize(direction))
{
}

RaycastResult rayPlaneTest(Ray ray, Plane plane)
{
	assert(isNormal(ray.direction));
	
	RaycastResult result;

	real32 rayDotNormal = dot(ray.direction, plane.normal);

	if (FLOAT_EQ(rayDotNormal, 0, EPSILON))
	{
		result.hit = false;
	}
	else
	{
		result.hit = true;
		result.t = dot(plane.point - ray.position, plane.normal) / rayDotNormal;
	}

	return result;
}

RaycastResult rayAabbTest(Ray ray, Aabb aabb)
{
	assert(isNormal(ray.direction));
	
	RaycastResult result;
	result.hit = false;

	Vec3 minPoint = aabb.center - aabb.halfDim;
	Vec3 maxPoint = aabb.center + aabb.halfDim;
	
	if (ray.direction.x > 0)
	{
		// test left face
		Plane leftFace = Plane(aabb.center - aabb.halfDim.x * Vec3(Axis3D::X), -Vec3(Axis3D::X));
		RaycastResult leftFaceResult = rayPlaneTest(ray, leftFace);
		if (leftFaceResult.hit)
		{
			Vec3 hitPoint = ray.position + ray.direction * leftFaceResult.t;
			if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
				hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
			{
				result = leftFaceResult;
				return result;
			}
		}
	}
	else
	{
		// test right face
		Plane rightFace = Plane(aabb.center + aabb.halfDim.x * Vec3(Axis3D::X), Vec3(Axis3D::X));
		RaycastResult rightFaceResult = rayPlaneTest(ray, rightFace);
		if (rightFaceResult.hit)
		{
			Vec3 hitPoint = ray.position + ray.direction * rightFaceResult.t;
			if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
				hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
			{
				result = rightFaceResult;
				return result;
			}
		}
	}

	if (ray.direction.y > 0)
	{
		// test bottom face
		Plane bottomFace = Plane(aabb.center - aabb.halfDim.y * Vec3(Axis3D::Y), -Vec3(Axis3D::Y));
		RaycastResult bottomFaceResult = rayPlaneTest(ray, bottomFace);
		if (bottomFaceResult.hit)
		{
			Vec3 hitPoint = ray.position + ray.direction * bottomFaceResult.t;
			if (hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x &&
				hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
			{
				result = bottomFaceResult;
				return result;
			}
		}
	}
	else
	{
		// test top face
		Plane topFace = Plane(aabb.center + aabb.halfDim.y * Vec3(Axis3D::Y), Vec3(Axis3D::Y));
		RaycastResult topFaceResult = rayPlaneTest(ray, topFace);
		if (topFaceResult.hit)
		{
			Vec3 hitPoint = ray.position + ray.direction * topFaceResult.t;
			if (hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x &&
				hitPoint.z >= minPoint.z && hitPoint.z <= maxPoint.z)
			{
				result = topFaceResult;
				return result;
			}
		}
	}

	if (ray.direction.z > 0)
	{
		// test back face
		Plane backFace = Plane(aabb.center - aabb.halfDim.z * Vec3(Axis3D::Z), -Vec3(Axis3D::Z));
		RaycastResult backFaceResult = rayPlaneTest(ray, backFace);
		if (backFaceResult.hit)
		{
			Vec3 hitPoint = ray.position + ray.direction * backFaceResult.t;
			if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
				hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x)
			{
				result = backFaceResult;
				return result;
			}
		}
	}
	else
	{
		// test front face
		Plane frontFace = Plane(aabb.center + aabb.halfDim.z * Vec3(Axis3D::Z), Vec3(Axis3D::Z));
		RaycastResult frontFaceResult = rayPlaneTest(ray, frontFace);
		if (frontFaceResult.hit)
		{
			Vec3 hitPoint = ray.position + ray.direction * frontFaceResult.t;
			if (hitPoint.y >= minPoint.y && hitPoint.y <= maxPoint.y &&
				hitPoint.x >= minPoint.x && hitPoint.x <= maxPoint.x)
			{
				result = frontFaceResult;
				return result;
			}
		}
	}

	return result;
}
