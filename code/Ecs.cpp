#include "GL/glew.h"

#include "Ecs.h"
#include "Scene.h"
#include "assert.h"

#include "DebugDraw.h"
#include "DebugGlobal.h"

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

Ecs::Ecs()
{
}

template<class T>
T* addComponent(Ecs::ComponentList<T>* componentList, Entity e)
{
	assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist
	assert(componentList->size < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	
	componentList->components[size] = componentToAdd;
	T* result = &(componentList->components[size]);

	componentList->size++;

	ComponentGroup<T> cg;
	cg.components = result;
	cg.numComponents = 1;
	componentList->lookup[e.id] = cg;
	
	return result;	
}

template<class T>
T* getComponent(Ecs::ComponentList<T>* componentList, Entity e)
{
	auto it = componentList->lookup.find(e.id);
	if (it == componentList->lookup.end())
	{
		return nullptr;
	}

	return it->second.components;
}

template<class T>
ComponentGroup<T> addComponents(Ecs::ComponentList<T>* componentList, Entity e, uint32 numComponents)
{
	assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist
	
	T* firstComponent = nullptr;

    for (uint32 i = 0; i < numComponents; i++)
    {
		assert(componentList->size < COMPONENT_ARRAY_SIZE);
		
        T component;
        component.entity = e;
		
        componentList->components[componentList->size] = component;
        if (i == 0)
        {
            firstComponent = &(componentList->components[componentList->size]);
        }
		
		componentList->size++;
    }

    ComponentGroup<T> cg;
	cg.entity = e;
    cg.components = firstComponent;
    cg.numComponents = numComponents;
    componentList->lookup[e.id] = cg;

    return cg;
}

template<class T>
ComponentGroup<T> getComponents(Ecs::ComponentList<T>* componentList, Entity e)
{
	auto it = componentList->lookup.find(e.id);
	if (it == componentList->lookup.end())
	{
		ComponentGroup<T> result;
		result.components = nullptr;
		result.numComponents = 0;
		return result;
	}

	return it->second;
}

Entity makeEntity(Ecs* ecs)
{
	Entity result;
	result.id = Ecs::nextEntityId;
	result.ecs = ecs;
	
	Ecs::nextEntityId++;
	return result;
}

TransformComponent* addTransformComponent(Entity e)
{
	return addComponent(e.ecs->transforms, e);
}

TransformComponent* getTransformComponent(Entity e)
{
	return getComponent(e.ecs->transforms, e);
}

CameraComponent* addCameraComponent(Entity e)
{
	return addComponent(e.ecs->cameras, e);
}

CameraComponent* getCameraComponent(Entity e)
{
	return getComponent( e.ecs->cameras, e);
}

PortalComponent* addPortalComponent(Entity e)
{
	return addComponent(e.ecs->portals, e);
}

PortalComponent* getPortalComponent(Entity e)
{
	return getComponent(e.ecs->portals, e);
}

DirectionalLightComponent* addDirectionalLightComponent(Entity e)
{
	return addComponent(e.ecs->directionalLights, e);
}

DirectionalLightComponent* getDirectionalLightComponent(Entity e)
{
	return getComponent(e.ecs->directionalLights, e);
}

PointLightComponent* addPointLightComponent(Entity e)
{
	return addComponent(e.ecs->pointLights, e);
}

PointLightComponent* getPointLightComponent(Entity e)
{
	return getComponent(e.ecs->pointLights, e);
}

ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents)
{
	return addComponents(e.ecs->pointLights, e, numComponents);
}

ComponentGroup<PointLightComponent> getPointLightComponents(Entity e)
{
	return getComponents(e.ecs->pointLights, e);
}

RenderComponent* addRenderComponent(Entity e)
{
	return addComponent(e.ecs->renderComponents, e);
}

RenderComponent* getRenderComponent(Entity e)
{
	return getComponent(e.ecs->renderComponents, e);
}

ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents)
{
	return addComponents(e.ecs->renderComponents, e, numComponents);
}

ComponentGroup<RenderComponent> getRenderComponents(Entity e)
{
	return getComponents(e.ecs->renderComponents, e);
}

void renderContentsOfAllPortals(Ecs* ecs, CameraComponent* camera, TransformComponent* cameraXfm, uint32 recursionLevel)
{
	if (recursionLevel > 0)
	{
		// TODO: handle recursive renders (i.e., looking through a portal through another portal)
		return;
	}


	for (uint32 i = 0; i < ecs->portals.size; i++)
	{
		//
		// Calculate the position and orientation of the camera sitting in the dest scene and looking "through" the portal
		// into the dest scene.
		//
		PortalComponent &pc = ecs->portals.components[i];

		Vec3 eyeToPortal = pc.sourceSceneXfm.position - cameraXfm->position;

		bool isLookingTowardsPortal = dot(eyeToPortal, pc.sourceSceneXfm.forward()) < 0;

		if (!isLookingTowardsPortal) continue;

		Quaternion intoSourcePortalOrientation = axisAngle(pc.sourceSceneXfm.up(), 180) * pc.sourceSceneXfm.orientation;
		Quaternion outOfDestPortalOrientation = pc.destSceneXfm.orientation;

		Quaternion transitionFromSourceToDest = relativeRotation(intoSourcePortalOrientation, outOfDestPortalOrientation);
		
		Vec3 transformedEyeToPortal = transitionFromSourceToDest * eyeToPortal;
		Vec3 portalViewpointPos = pc.destSceneXfm.position - transformedEyeToPortal;

		Quaternion portalViewpointOrientation = transitionFromSourceToDest * cameraXfm->orientation;
		
		TransformComponent portalViewpointXfm(portalViewpointPos, portalViewpointOrientation);

		glEnable(GL_STENCIL_TEST);
		{
		
			//
			// Render the portal in the source scene and write to stencil buffer
			//
			glDepthMask(GL_FALSE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			{
				
				//
				// Render to stencil
				//
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilMask(0xFF);
			
				Shader* portalShader = portalShader();
				uint32 portalVao = quadVao();

				Mat4 model = modelToWorld(&pc.sourceSceneXfm);
				Mat4 view = worldToView(cameraXfm);
				Mat4 projection = camera->projectionMatrix;

				portalShader->bind();
				portalShader->setMat4("model", model);
				portalShader->setMat4("view", view);
				portalShader->setMat4("projection", projection);
				portalShader->setVec3("debugColor", Vec3(0, 1, 0));

				glBindVertexArray(portalVao);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);

			}
			glDepthMask(GL_TRUE);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			//
			// Render from portal camera
			//
			{
				glStencilFunc(GL_EQUAL, 1, 0xFF);
				glStencilMask(0x00);
		
				if (!debug_hidePortalContents)
				{
					glClear(GL_DEPTH_BUFFER_BIT);
					renderScene(pc.destScene, camera, &portalViewpointXfm, recursionLevel + 1, &pc.destSceneXfm);
				}
			}
		}
		glDisable(GL_STENCIL_TEST);

			
		//
		// Render the portal's depth to the depth buffer
		//
		glDepthFunc(GL_ALWAYS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		{
			Shader* portalShader = portalSshader();
			uint32 portalVao = quadVao();

			Mat4 model = modelToWorld(&pc.sourceSceneXfm);
			Mat4 view = worldToView(cameraXfm);
			Mat4 projection = camera->projectionMatrix;

			portalShader->bind();
			portalShader->setMat4("model", model);
			portalShader->setMat4("view", view);
			portalShader->setMat4("projection", projection);
			portalShader->setVec3("debugColor", Vec3(0, 1, 0));

			glBindVertexArray(portalVao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
		glDepthFunc(GL_LESS);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);

		DebugDraw::instance().drawAARect3(pc.sourceSceneXfm.position, Vec3(pc.dimensions(), 0.2), camera, cameraXfm);
	}
}

void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, TransformComponent* cameraXfm, bool renderingViaPortal, TransformComponent* destPortalXfm)
{
	for (uint32 i = 0; i < ecs->renderComponents.size; i++)
	{
		RenderComponent &rc = ecs->renderComponents.components[i];
		TransformComponent* xfm = getTransformComponent(rc.entity);

		if (renderingViaPortal)
		{
			bool behindDestPortal = dot(destPortalXfm->forward(), destPortalXfm->position - xfm->position) > 0;
			if (behindDestPortal) continue;
		}

		assert(xfm != nullptr); // Render component cannot exist without corresponding transform component
		if (xfm == nullptr) continue;

		// TODO: pass closest lights to RenderComponent draw...
		// ECS should be responsible for finding the light, but render component should be responsible
		// for handling the lighting
		if (rc.material->receiveLight)
		{
			PointLightComponent* pl = closestPointLight(xfm);
			Shader* shader = rc.material->shader;
			shader->bind();

			if (pl != nullptr)
			{
				TransformComponent* plXfm = getTransformComponent(pl->entity);

			
				shader->setVec3("pointLights[0].posWorld", plXfm->position);
				shader->setVec3("pointLights[0].intensity", pl->intensity);
				shader->setFloat("pointLights[0].attenuationConstant", pl->attenuationConstant);
				shader->setFloat("pointLights[0].attenuationLinear", pl->attenuationLinear);
				shader->setFloat("pointLights[0].attenuationQuadratic", pl->attenuationQuadratic);
			}

			for (uint32 j = 0; j < directionalLights.size; j++)
			{
				// TODO: what happens if the number of directional lights exceeds the number allowed in the shader?
				// How can we guarantee it doesnt? Should we just hard code a limit that is the same as the limit
				// in the shader? Is that robust when we change the shader?
				DirectionalLightComponent* dlc = directionalLights.components + j;

				std::string directionVarName = "directionalLights[" + std::to_string(j) + "].direction";
				std::string intensityVarName = "directionalLights[" + std::to_string(j) + "].intensity";
				
				shader->setVec3(directionVarName, dlc->direction);
				shader->setVec3(intensityVarName, dlc->intensity);
			}
		}

		drawRenderComponent(&rc, xfm, camera, cameraXfm);
	}
}

PointLightComponent* closestPointLight(TransformComponent* xfm)
{
	Ecs* ecs = xfm->entity.ecs;
	
	PointLightComponent* closest = nullptr;
	real32 closestDistance = FLT_MAX;

	for (uint32 i = 0; i < ecs->pointLights.size; i++)
	{
		PointLightComponent* pl = &(ecs->pointLights.components[i]);
		TransformComponent* plXfm = getTransformComponent(pl->entity);

		assert(plXfm != nullptr);
		if (plXfm == nullptr) continue;
		
		real32 distance = distance(xfm->position, plXfm->position);

		if (distance < closestDistance)
		{
			closestDistance = distance;
			closest = pl;
		}
	}

	return closest;
}
