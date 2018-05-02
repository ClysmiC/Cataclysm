#include "Ecs.h"
#include "assert.h"

TransformComponent* Ecs::addTransformComponent(Entity e)
{
	return addComponent(e, transformComponents, transformComponentLookup);
}

TransformComponent* Ecs::getTransformComponent(Entity e)
{
	return getComponent(e, transformComponents, transformComponentLookup);
}

RenderComponent* Ecs::addRenderComponent(Entity e)
{
	return addComponent(e, renderComponents, renderComponentLookup);
}

RenderComponent* Ecs::getRenderComponent(Entity e)
{
	return getComponent(e, renderComponents, renderComponentLookup);
}

void Ecs::RenderAllRenderComponents()
{
	for (RenderComponent rc : renderComponents)
	{
		TransformComponent* tc = getTransformComponent(rc.entity);

		assert(tc != nullptr); // Render component cannot exist without corresponding transform component
		if (tc == nullptr) continue;

		rc.ncTransform = *tc;
		rc.draw();
	}
}

uint32 Ecs::nextEntityId()
{
	return nextEntityId_++;
}

template<class T>
T* Ecs::addComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> lookup)
{
	assert(lookup.find(e) == lookup.end()); // doesnt already exist

	T componentToAdd;
	
	components.push_back(componentToAdd);
	T* result = &components.back();

	lookup[e] = result;
	return result;	
}

template<class T>
T* Ecs::getComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> lookup)
{
	auto it = lookup.find(e);
	if (it == lookup.end())
	{
		return nullptr;
	}

	return it->second;
}
