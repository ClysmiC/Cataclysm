#include "Ecs.h"
#include "assert.h"

Ecs::Ecs()
{
}

template<class T>
T* ComponentList<T>::addComponent(Entity e)
{
	assert(lookup.find(e) == lookup.end()); // doesnt already exist
	assert(nextIndex < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	
	components[nextIndex] = componentToAdd;
	T* result = &(components[nextIndex]);

	nextIndex++;

	lookup[e] = result;
	return result;	
}

template<class T>
T* ComponentList<T>::getComponent(Entity e)
{
	auto it = lookup.find(e);
	if (it == lookup.end())
	{
		return nullptr;
	}

	return it->second;
}

TransformComponent* Ecs::addTransformComponent(Entity e)
{
	return transforms.addComponent(e);
}

TransformComponent* Ecs::getTransformComponent(Entity e)
{
	return transforms.getComponent(e);
}

PointLightComponent* Ecs::addPointLightComponent(Entity e)
{
	return pointLights.getComponent(e);
}

PointLightComponent* Ecs::getPointLightComponent(Entity e)
{
	return pointLights.getComponent(e);
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
