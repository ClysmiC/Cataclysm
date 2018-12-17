#include "ecs/components/CameraComponent.h"
#include "ecs/Ecs.h"
#include "Transform.h"
#include "TransformComponent.h"

ITransform* CameraComponent::getTransform()
{
    return getComponent<TransformComponent>(this->entity);
}
