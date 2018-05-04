#include "Ecs.h"
#include "assert.h"

Ecs::Ecs()
{
}

TransformComponent* Ecs::addTransformComponent(Entity e)
{
	return addComponent(e, transformComponents, transformComponentNextIndex, transformComponentLookup);
}

TransformComponent* Ecs::getTransformComponent(Entity e)
{
	return getComponent(e, transformComponentLookup);
}

RenderComponentCollection Ecs::addRenderComponents(Entity e, uint32 numComponents)
{
    RenderComponent* firstRenderComponent = nullptr;

    for (uint32 i = 0; i < numComponents; i++)
    {
        RenderComponent rc;
        rc.entity = e;
        renderComponents[renderComponentNextIndex] = rc;
        if (i == 0)
        {
            firstRenderComponent = &(renderComponents[renderComponentNextIndex]);
        }
		
		renderComponentNextIndex++;
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

void Ecs::renderAllRenderComponents(Camera& camera)
{
	for (uint32 i = 0; i < renderComponentNextIndex; i++)
	{
		RenderComponent &rc = renderComponents[i];
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
T* Ecs::addComponent(Entity e, T* components, uint32 &index, std::unordered_map<Entity, T*> &lookup)
{
	assert(lookup.find(e) == lookup.end()); // doesnt already exist
	assert(index < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	
	components[index] = componentToAdd;
	T* result = &(components[index]);

	index++;

	lookup[e] = result;
	return result;	
}

template<class T>
T* Ecs::getComponent(Entity e, std::unordered_map<Entity, T*> &lookup)
{
	auto it = lookup.find(e);
	if (it == lookup.end())
	{
		return nullptr;
	}

	return it->second;
}
