#include "Ecs.h"
#include "assert.h"

Ecs::Ecs()
{
}

template<class T>
T*
ComponentList<T>::addComponent(Entity e)
{
	assert(lookup.find(e) == lookup.end()); // doesnt already exist
	assert(size < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	
	components[size] = componentToAdd;
	T* result = &(components[size]);

	size++;

	lookup[e] = result;
	return result;	
}

template<class T>
T*
ComponentList<T>::getComponent(Entity e)
{
	auto it = lookup.find(e);
	if (it == lookup.end())
	{
		return nullptr;
	}

	return it->second;
}

TransformComponent*
Ecs::addTransformComponent(Entity e)
{
	return transforms.addComponent(e);
}

TransformComponent*
Ecs::getTransformComponent(Entity e)
{
	return transforms.getComponent(e);
}

PointLightComponent*
Ecs::addPointLightComponent(Entity e)
{
	return pointLights.addComponent(e);
}

PointLightComponent*
Ecs::getPointLightComponent(Entity e)
{
	return pointLights.getComponent(e);
}

RenderComponentCollection
Ecs::addRenderComponents(Entity e, uint32 numComponents)
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

RenderComponentCollection
Ecs::getRenderComponents(Entity e)
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

void
Ecs::renderAllRenderComponents(TransformComponent* cameraXfm)
{
	for (uint32 i = 0; i < renderComponentNextIndex; i++)
	{
		RenderComponent &rc = renderComponents[i];
		TransformComponent* xfm = getTransformComponent(rc.entity);

		assert(xfm != nullptr); // Render component cannot exist without corresponding transform component
		if (xfm == nullptr) continue;

		// TODO: pass closest lights to RenderComponent draw...
		// ECS should be responsible for finding the light, but render component should be responsible
		// for handling the lighting
		if (rc.material->receiveLight)
		{
			PointLightComponent* pl = closestPointLight(xfm);
			TransformComponent* plXfm = getTransformComponent(pl->entity);

			Shader* shader = rc.material->shader;
			
			shader->setVec3("pointLights[0].posWorld", plXfm->position());
			shader->setVec3("pointLights[0].intensity", pl->intensity);
			shader->setFloat("pointLights[0].attenuationConstant", pl->attenuationConstant);
			shader->setFloat("pointLights[0].attenuationLinear", pl->attenuationLinear);
			shader->setFloat("pointLights[0].attenuationQuadratic", pl->attenuationQuadratic);
		}

		rc.draw(xfm, cameraXfm);
	}
}

PointLightComponent*
Ecs::closestPointLight(TransformComponent* xfm)
{
	PointLightComponent* closest = nullptr;
	real32 closestDistance = FLT_MAX;

	for (uint32 i = 0; i < pointLights.size; i++)
	{
		PointLightComponent* pl = &(pointLights.components[i]);
		TransformComponent* plXfm = getTransformComponent(pl->entity);

		assert(plXfm != nullptr);
		if (plXfm == nullptr) continue;
		
		real32 distance = xfm->distance(plXfm);

		if (distance < closestDistance)
		{
			closestDistance = distance;
			closest = pl;
		}
	}

	return closest;
}

uint32
Ecs::nextEntityId()
{
	return nextEntityId_++;
}
