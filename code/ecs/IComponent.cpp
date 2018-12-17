#include "IComponent.h"

IComponent::IComponent(Entity entity)
{
    this->entity = entity;
}

void IComponent::onAddComponent() {}
void IComponent::onRemoveComponent() {}
