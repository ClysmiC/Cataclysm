#pragma once

#include "Component.h"
#include "als_math.h"

enum ColliderType
{
	RECT3,
	SPHERE,
	CYLINDER,
	CAPSULE,
};

struct ColliderComponent : public Component
{
	ColliderComponent();
	
	Vec3 xfmOffset;
	ColliderType type;

	union
	{
		// Sphere, Cylinder, and Capsule types
		struct
		{
			real32 length; // unused for sphere
			real32 radius;
			Axis3D axis; 
		};

		struct
		{
			real32 xLength; // use 'length' to address this memory
			real32 yLength;
			real32 zLength;
		};
	};
};

Vec3 colliderCenter(ColliderComponent* collider);
Vec3 scaledXfmOffset(ColliderComponent* collider);
real32 scaledLength(ColliderComponent* collider);
real32 scaledRadius(ColliderComponent* collider);

real32 scaledXLength(ColliderComponent* collider);
real32 scaledYLength(ColliderComponent* collider);
real32 scaledZLength(ColliderComponent* collider);

bool pointInsideCollider(ColliderComponent* collider, Vec3 point);
