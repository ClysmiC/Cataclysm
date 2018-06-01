#include "GL/glew.h"

#include "Ecs.h"
#include "Scene.h"
#include "assert.h"

#include "DebugGlobal.h"

uint32 Ecs::nextEntityId = 1;

Ecs::Ecs()
{
}

template<class T>
T*
Ecs::ComponentList<T>::addComponent(Entity e, Ecs* ecs)
{
	assert(lookup.find(e.id) == lookup.end()); // doesnt already exist
	assert(size < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	componentToAdd.ecs = ecs;
	
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
Ecs::ComponentList<T>::addComponents(Entity e, uint32 numComponents, Ecs* ecs)
{
	assert(lookup.find(e.id) == lookup.end()); // doesnt already exist
	
	T* firstComponent = nullptr;

    for (uint32 i = 0; i < numComponents; i++)
    {
		assert(size < COMPONENT_ARRAY_SIZE);
		
        T component;
        component.entity = e;
		component.ecs = ecs;
		
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
	return transforms.addComponent(e, this);
}

TransformComponent*
Ecs::getTransformComponent(Entity e)
{
	return transforms.getComponent(e);
}

CameraComponent*
Ecs::addCameraComponent(Entity e)
{
	return cameras.addComponent(e, this);
}

CameraComponent*
Ecs::getCameraComponent(Entity e)
{
	return cameras.getComponent(e);
}

PortalComponent*
Ecs::addPortalComponent(Entity e)
{
	return portals.addComponent(e, this);
}

PortalComponent*
Ecs::getPortalComponent(Entity e)
{
	return portals.getComponent(e);
}

PointLightComponent*
Ecs::addPointLightComponent(Entity e)
{
	return pointLights.addComponent(e, this);
}

PointLightComponent*
Ecs::getPointLightComponent(Entity e)
{
	return pointLights.getComponent(e);
}

ComponentGroup<PointLightComponent>
Ecs::addPointLightComponents(Entity e, uint32 numComponents)
{
	return pointLights.addComponents(e, numComponents, this);
}

ComponentGroup<PointLightComponent>
Ecs::getPointLightComponents(Entity e)
{
	return pointLights.getComponents(e);
}

RenderComponent*
Ecs::addRenderComponent(Entity e)
{
	return renderComponents.addComponent(e, this);
}

RenderComponent*
Ecs::getRenderComponent(Entity e)
{
	return renderComponents.getComponent(e);
}

ComponentGroup<RenderComponent>
Ecs::addRenderComponents(Entity e, uint32 numComponents)
{
	return renderComponents.addComponents(e, numComponents, this);
}

ComponentGroup<RenderComponent>
Ecs::getRenderComponents(Entity e)
{
	return renderComponents.getComponents(e);
}

void
Ecs::renderContentsOfAllPortals(CameraComponent* camera, TransformComponent* cameraXfm)
{
	// 1. Render the portal rectangles to the stencil buffer
	// 2. Render the scenes that the portals are looking into
	//    but only where the the stencil is set from 1


	for (uint32 i = 0; i < portals.size; i++)
	{
		//
		// Calculate the position and orientation of the camera sitting in the dest scene and looking "through" the portal
		// into the dest scene.
		//
		PortalComponent &pc = portals.components[i];

		Vec3 eyeToPortal = pc.sourceSceneXfm.position() - cameraXfm->position();

		Quaternion intoSourcePortalOrientation = axisAngle(pc.sourceSceneXfm.up(), 180) * pc.sourceSceneXfm.orientation();
		Quaternion outOfDestPortalOrientation = pc.destSceneXfm.orientation();

		Quaternion transitionFromSourceToDest = relativeRotation(intoSourcePortalOrientation, outOfDestPortalOrientation);
		
		Vec3 transformedEyeToPortal = transitionFromSourceToDest * eyeToPortal;
		Vec3 portalViewpointPos = pc.destSceneXfm.position() - transformedEyeToPortal;

		Quaternion portalViewpointOrientation = transitionFromSourceToDest * cameraXfm->orientation();
		
		TransformComponent portalViewpointXfm(portalViewpointPos, portalViewpointOrientation);

		glEnable(GL_STENCIL_TEST);
		{
		
			//
			// Render the portal in the source scene and write to stencil buffer
			//
			{
				//
				// Render to stencil
				//
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilMask(0xFF);
			
				Shader* portalShader = PortalComponent::shader();
				uint32 portalVao = PortalComponent::quadVao();

				Mat4 model = pc.sourceSceneXfm.modelToWorld();
				Mat4 view = cameraXfm->worldToView();
				Mat4 projection = camera->projectionMatrix;

				portalShader->bind();
				portalShader->setMat4("model", model);
				portalShader->setMat4("view", view);
				portalShader->setMat4("projection", projection);
				portalShader->setVec3("debugColor", Vec3(0, 1, 0));

				glBindVertexArray(pc.quadVao());
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}

			// (DEBUG)
			// Render the portal in the dest scene (if it is the same)
			// as the source scene
			//
			if (pc.destScene == pc.ecs->scene)
			{
				Shader* portalShader = PortalComponent::shader();
				uint32 portalVao = PortalComponent::quadVao();

				Mat4 model = pc.destSceneXfm.modelToWorld();
				Mat4 view = cameraXfm->worldToView();
				Mat4 projection = camera->projectionMatrix;

				portalShader->bind();
				portalShader->setMat4("model", model);
				portalShader->setMat4("view", view);
				portalShader->setMat4("projection", projection);
				portalShader->setVec3("debugColor", Vec3(1, 0, 0));

				glBindVertexArray(pc.quadVao());
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}

		

			//
			// Render from portal camera
			//
			{
				glStencilFunc(GL_EQUAL, 1, 0xFF);
				glStencilMask(0x00);
		
				if (!debug_hidePortalContents)
				{
					pc.destScene->ecs->renderAllRenderComponents(camera, &portalViewpointXfm);
				}
			}
		}
		glDisable(GL_STENCIL_TEST);
		
		glClear(GL_STENCIL_BUFFER_BIT);
	}
}

void
Ecs::renderAllRenderComponents(CameraComponent* camera, TransformComponent* cameraXfm)
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
			shader->bind();
			
			shader->setVec3("pointLights[0].posWorld", plXfm->position());
			shader->setVec3("pointLights[0].intensity", pl->intensity);
			shader->setFloat("pointLights[0].attenuationConstant", pl->attenuationConstant);
			shader->setFloat("pointLights[0].attenuationLinear", pl->attenuationLinear);
			shader->setFloat("pointLights[0].attenuationQuadratic", pl->attenuationQuadratic);
		}

		rc.draw(xfm, camera, cameraXfm);
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
