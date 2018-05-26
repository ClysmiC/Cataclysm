#include "Entity.h"
#include "Ecs.h"

CameraEntity::CameraEntity(){}
CameraEntity::CameraEntity(Entity entity)
{
	init(entity);
}

void
CameraEntity::init(Entity entity)
{
	id = entity.id;
	ecs = entity.ecs;

	transformComponent = entity.ecs->getTransformComponent(entity);
	cameraComponent = entity.ecs->getCameraComponent(entity);
}
