#include "Ecs.h"
#include "assert.h"

Ecs::Ecs()
{
    // TODO: solve this issue.
    transformComponents.reserve(10000);
    renderComponents.reserve(10000);
}

TransformComponent* Ecs::addTransformComponent(Entity e)
{
	return addComponent(e, transformComponents, transformComponentLookup);
}

TransformComponent* Ecs::getTransformComponent(Entity e)
{
	return getComponent(e, transformComponents, transformComponentLookup);
}

RenderComponentCollection Ecs::addRenderComponents(Entity e, uint32 numComponents)
{
    RenderComponent* firstRenderComponent = nullptr;

    for (uint32 i = 0; i < numComponents; i++)
    {
        RenderComponent rc;
        rc.entity = e;
        renderComponents.push_back(rc);
        
        if (i == 0)
        {
            firstRenderComponent = &renderComponents.back();
        }
    }

    RenderComponentCollection rcc;
    rcc.numComponents = numComponents;
    rcc.renderComponents = firstRenderComponent;

    renderComponentLookup[e] = rcc;

    return rcc;
}

RenderComponentCollection Ecs::getRenderComponents(Entity e)
{
    RenderComponentCollection result;

	auto it = renderComponentLookup.find(e);
	if (it == renderComponentLookup.end())
	{
		result.numComponents = 0;
        result.renderComponents = nullptr;
	}
    else
    {
        result = it->second;
    }

	return result;
}

void Ecs::RenderAllRenderComponents(Camera& camera)
{
	for (RenderComponent rc : renderComponents)
	{
		TransformComponent* tc = getTransformComponent(rc.entity);

		assert(tc != nullptr); // Render component cannot exist without corresponding transform component
		if (tc == nullptr) continue;

		rc.draw(tc, camera);
	}
}

uint32 Ecs::nextEntityId()
{
	return nextEntityId_++;
}

template<class T>
T* Ecs::addComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> &lookup)
{
	assert(lookup.find(e) == lookup.end()); // doesnt already exist

	T componentToAdd;
    componentToAdd.entity = e;
	
	components.push_back(componentToAdd);
	T* result = &components.back();

	lookup[e] = result;
	return result;	
}

template<class T>
T* Ecs::getComponent(Entity e, std::vector<T> &components, std::unordered_map<Entity, T*> &lookup)
{
	auto it = lookup.find(e);
	if (it == lookup.end())
	{
		return nullptr;
	}

	return it->second;
}
