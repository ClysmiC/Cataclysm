#include "GL/glew.h"

#include "Ecs.h"
#include "Scene.h"
#include "assert.h"

#include "Quad.h"
#include "DebugDraw.h"
#include "DebugGlobal.h"

#include "Transform.h"

#include "Game.h"

// ID 0 is a null entity
uint32 Ecs::nextEntityId = 1;

template<class T>
T* addComponent(Ecs::ComponentList<T>* componentList, Entity e)
{
	assert(componentList->lookup.find(e.id) == componentList->lookup.end()); // doesnt already exist
	assert(componentList->size < COMPONENT_ARRAY_SIZE);

	T componentToAdd;
    componentToAdd.entity = e;
	
	componentList->components[componentList->size] = componentToAdd;
	T* result = &(componentList->components[componentList->size]);

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

Entity makeEntity(Ecs* ecs, std::string friendlyName)
{
	Entity result;
	result.id = Ecs::nextEntityId;
	result.ecs = ecs;
	result.friendlyName = friendlyName;
	
	Ecs::nextEntityId++;
	return result;
}

TransformComponent* addTransformComponent(Entity e)
{
	return addComponent(&e.ecs->transforms, e);
}

TransformComponent* getTransformComponent(Entity e)
{
	return getComponent(&e.ecs->transforms, e);
}

CameraComponent* addCameraComponent(Entity e)
{
	return addComponent(&e.ecs->cameras, e);
}

CameraComponent* getCameraComponent(Entity e)
{
	return getComponent(&e.ecs->cameras, e);
}

PortalComponent* addPortalComponent(Entity e)
{
	return addComponent(&e.ecs->portals, e);
}

PortalComponent* getPortalComponent(Entity e)
{
	return getComponent(&e.ecs->portals, e);
}

DirectionalLightComponent* addDirectionalLightComponent(Entity e)
{
	return addComponent(&e.ecs->directionalLights, e);
}

DirectionalLightComponent* getDirectionalLightComponent(Entity e)
{
	return getComponent(&e.ecs->directionalLights, e);
}

PointLightComponent* addPointLightComponent(Entity e)
{
	return addComponent(&e.ecs->pointLights, e);
}

PointLightComponent* getPointLightComponent(Entity e)
{
	return getComponent(&e.ecs->pointLights, e);
}

ComponentGroup<PointLightComponent> addPointLightComponents(Entity e, uint32 numComponents)
{
	return addComponents(&e.ecs->pointLights, e, numComponents);
}

ComponentGroup<PointLightComponent> getPointLightComponents(Entity e)
{
	return getComponents(&e.ecs->pointLights, e);
}

RenderComponent* addRenderComponent(Entity e)
{
	return addComponent(&e.ecs->renderComponents, e);
}

RenderComponent* getRenderComponent(Entity e)
{
	return getComponent(&e.ecs->renderComponents, e);
}

ComponentGroup<RenderComponent> addRenderComponents(Entity e, uint32 numComponents)
{
	return addComponents(&e.ecs->renderComponents, e, numComponents);
}

ComponentGroup<RenderComponent> getRenderComponents(Entity e)
{
	return getComponents(&e.ecs->renderComponents, e);
}

ColliderComponent* addColliderComponent(Entity e)
{
	return addComponent(&e.ecs->colliders, e);
}

ColliderComponent* getColliderComponent(Entity e)
{
	return getComponent(&e.ecs->colliders, e);
}

ComponentGroup<ColliderComponent> addColliderComponents(Entity e, uint32 numComponents)
{
	return addComponents(&e.ecs->colliders, e, numComponents);
}

ComponentGroup<ColliderComponent> getColliderComponents(Entity e)
{
	return getComponents(&e.ecs->colliders, e);
}

void renderContentsOfAllPortals(Scene* scene, CameraComponent* camera, Transform* cameraXfm, uint32 recursionLevel)
{
	if (recursionLevel > 0)
	{
		// TODO: handle recursive renders (i.e., looking through a portal through another portal)
		return;
	}

	std::vector<PortalComponent*> portals = portalsInScene(scene);

	for (uint32 i = 0; i < portals.size(); i++)
	{
		//
		// Calculate the position and orientation of the camera sitting in the dest scene and looking "through" the portal
		// into the dest scene.
		//
		PortalComponent* pc = portals[i];

		Transform* sourceSceneXfm = getSourceSceneXfm(pc, scene);
		Transform* destSceneXfm = getDestSceneXfm(pc, scene);
		
		Vec3 eyeToPortal = sourceSceneXfm->position - cameraXfm->position;

		bool isLookingTowardsPortal = dot(eyeToPortal, sourceSceneXfm->forward()) < 0;

		if (!isLookingTowardsPortal) continue;

		Quaternion intoSourcePortalOrientation = axisAngle(sourceSceneXfm->up(), 180) * sourceSceneXfm->orientation;
		Quaternion outOfDestPortalOrientation = destSceneXfm->orientation;

		Quaternion transitionFromSourceToDest = relativeRotation(intoSourcePortalOrientation, outOfDestPortalOrientation);
		
		Vec3 transformedEyeToPortal = transitionFromSourceToDest * eyeToPortal;
		Vec3 portalViewpointPos = destSceneXfm->position - transformedEyeToPortal;

		Quaternion portalViewpointOrientation = transitionFromSourceToDest * cameraXfm->orientation;
		
		Transform portalViewpointXfm(portalViewpointPos, portalViewpointOrientation);

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
			
				Shader* shader = portalShader();
				uint32 portalVao = quadVao();

				Mat4 model = modelToWorld(sourceSceneXfm);
				Mat4 view = worldToView(cameraXfm);
				Mat4 projection = camera->projectionMatrix;

				bind(shader);
				setMat4(shader, "model", model);
				setMat4(shader, "view", view);
				setMat4(shader, "projection", projection);
				setVec3(shader, "debugColor", Vec3(0, 1, 0));

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
					renderScene(getDestScene(pc, scene), camera, &portalViewpointXfm, recursionLevel + 1, destSceneXfm);
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
			Shader* shader = portalShader();
			uint32 portalVao = quadVao();

			Mat4 model = modelToWorld(sourceSceneXfm);
			Mat4 view = worldToView(cameraXfm);
			Mat4 projection = camera->projectionMatrix;

			bind(shader);
			setMat4(shader, "model", model);
			setMat4(shader, "view", view);
			setMat4(shader, "projection", projection);
			setVec3(shader, "debugColor", Vec3(0, 1, 0));

			glBindVertexArray(portalVao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
		glDepthFunc(GL_LESS);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);

		DebugDraw::instance().drawAARect3(sourceSceneXfm->position, Vec3(getDimensions(pc), 0.2), camera, cameraXfm);
	}
}

void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, Transform* cameraXfm, bool renderingViaPortal, Transform* destPortalXfm)
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
			bind(shader);

			if (pl != nullptr)
			{
				TransformComponent* plXfm = getTransformComponent(pl->entity);
			
				setVec3(shader, "pointLights[0].posWorld", plXfm->position);
				setVec3(shader, "pointLights[0].intensity", pl->intensity);
				setFloat(shader, "pointLights[0].attenuationConstant", pl->attenuationConstant);
				setFloat(shader, "pointLights[0].attenuationLinear", pl->attenuationLinear);
				setFloat(shader, "pointLights[0].attenuationQuadratic", pl->attenuationQuadratic);
			}

			for (uint32 j = 0; j < ecs->directionalLights.size; j++)
			{
				// TODO: what happens if the number of directional lights exceeds the number allowed in the shader?
				// How can we guarantee it doesnt? Should we just hard code a limit that is the same as the limit
				// in the shader? Is that robust when we change the shader?
				DirectionalLightComponent* dlc = ecs->directionalLights.components + j;

				std::string directionVarName = "directionalLights[" + std::to_string(j) + "].direction";
				std::string intensityVarName = "directionalLights[" + std::to_string(j) + "].intensity";
				
				setVec3(shader, directionVarName, dlc->direction);
				setVec3(shader, intensityVarName, dlc->intensity);
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
		
		real32 dist = distance(xfm->position, plXfm->position);

		if (dist < closestDistance)
		{
			closestDistance = dist;
			closest = pl;
		}
	}

	return closest;
}
