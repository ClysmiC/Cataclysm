#include "Ecs.h"
#include "assert.h"

Ecs::Ecs()
{
}

template<class T>
T*
Ecs::ComponentList<T>::addComponent(Entity e)
{
	assert(lookup.find(e) == lookup.end()); // doesnt already exist
	assert(size < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	componentToAdd.ecs = this;
	
	components[size] = componentToAdd;
	T* result = &(components[size]);

	size++;

	ComponentGroup<T> cg;
	cg.components = result;
	cg.numComponents = 1;
	lookup[e.id] = cg;
	
	return result;	
}

template<class T>
T*
Ecs::ComponentList<T>::getComponent(Entity e)
{
	auto it = lookup.find(e.id);
	if (it == lookup.end())
	{
		return nullptr;
	}

	return it->second.components;
}

template<class T>
ComponentGroup<T>
Ecs::ComponentList<T>::addComponents(Entity e, uint32 numComponents)
{
	assert(lookup.find(e.id) == lookup.end()); // doesnt already exist
	
	T* firstComponent = nullptr;

    for (uint32 i = 0; i < numComponents; i++)
    {
		assert(size < COMPONENT_ARRAY_SIZE);
		
        T component;
        component.entity = e;
		component.ecs = this;
		
        components[size] = component;
        if (i == 0)
        {
            firstComponent = &(components[size]);
        }
		
		size++;
    }

    ComponentGroup<T> cg;
	cg.entity = e;
    cg.components = firstComponent;
    cg.numComponents = numComponents;
    lookup[e.id] = cg;

    return cg;
}

template<class T>
ComponentGroup<T>
Ecs::ComponentList<T>::getComponents(Entity e)
{
	auto it = lookup.find(e.id);
	if (it == lookup.end())
	{
		ComponentGroup<T> result;
		result.components = nullptr;
		result.numComponents = 0;
		return result;
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

ComponentGroup<PointLightComponent>
Ecs::addPointLightComponents(Entity e, uint32 numComponents)
{
	return pointLights.addComponents(e, numComponents);
}

ComponentGroup<PointLightComponent>
Ecs::getPointLightComponents(Entity e)
{
	return pointLights.getComponents(e);
}

RenderComponent*
Ecs::addRenderComponent(Entity e)
{
	return renderComponents.addComponent(e);
}

RenderComponent*
Ecs::getRenderComponent(Entity e)
{
	return renderComponents.getComponent(e);
}

ComponentGroup<RenderComponent>
Ecs::addRenderComponents(Entity e, uint32 numComponents)
{
	return renderComponents.addComponents(e, numComponents);
}

ComponentGroup<RenderComponent>
Ecs::getRenderComponents(Entity e)
{
	return renderComponents.getComponents(e);
}

void
Ecs::renderAllRenderComponents(CameraComponent* camera)
{
	for (uint32 i = 0; i < renderComponents.size; i++)
	{
		RenderComponent &rc = renderComponents.components[i];
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

		rc.draw(xfm, camera);
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

Entity
Ecs::makeEntity()
{
	Entity result;
	result.id = nextEntityId;
	result.ecs = this;
	
	nextEntityId++;
	return result;
}
