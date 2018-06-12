#include "ColliderComponent.h"
#include <algorithm>

#include "TransformComponent.h"
#include "Ecs.h"

Vec3 scaledXfmOffset(ColliderComponent* collider)
{
	TransformComponent *xfm = getTransformComponent(collider->entity);
	return hadamard(collider->xfmOffset, xfm->scale);
}
real32 scaledLength(ColliderComponent* collider)
{
	assert(collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE);
	
	TransformComponent *xfm = getTransformComponent(collider->entity);
	
	real32 scaleValue = getValue(xfm->scale, collider->axis);
	return collider->length * scaleValue;
}

real32 scaledRadius(ColliderComponent* collider)
{
	assert(collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE || collider->type == ColliderType::SPHERE);

	TransformComponent *xfm = getTransformComponent(collider->entity);
	real32 scaleValue = std::max(std::max(xfm->scale.x, xfm->scale.y), xfm->scale.z);

	return scaleValue * collider->radius;
}

real32 scaledXLength(ColliderComponent* collider)
{
	assert(collider->type == ColliderType::RECT3);
	
	TransformComponent *xfm = getTransformComponent(collider->entity);
	return xfm->scale.x * collider->xLength;
}

real32 scaledYLength(ColliderComponent* collider)
{
	assert(collider->type == ColliderType::RECT3);
	
	TransformComponent *xfm = getTransformComponent(collider->entity);
	return xfm->scale.y * collider->yLength;
}

real32 scaledZLength(ColliderComponent* collider)
{
	assert(collider->type == ColliderType::RECT3);
	
	TransformComponent *xfm = getTransformComponent(collider->entity);
	return xfm->scale.z * collider->zLength;
}
