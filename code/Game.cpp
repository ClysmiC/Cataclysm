#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <thread>

#include "Ecs.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "DebugDraw.h"
#include "Mesh.h"
#include "GL/glew.h"

#include "DebugGlobal.h"

bool keys[1024];
bool lastKeys[1024];

float mouseX;
float mouseY;
float mouseXPrev;
float mouseYPrev;

real32 deltaTMs;

void updateLastKeys()
{
	for (uint32 i = 0; i < 1024; i++)
	{
		lastKeys[i] = keys[i];
	}
}

Scene buildTestScene1()
{
	Scene result;
	
	ResourceManager& rm = ResourceManager::instance();
	
	Mesh *hexMesh = rm.initMesh("hex/hex.obj", true, true);
	Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, true);

	// Set up light
	{
		Entity light = makeEntity(result.ecs, "light");
        Mesh* m = bulb;
        
        TransformComponent *tc = addTransformComponent(light);
        tc->position = Vec3(0, 0, -2);
        tc->scale = Vec3(.35);
		
        ComponentGroup<RenderComponent> rcc = addRenderComponents(light, m->submeshes.size());

        for(uint32 i = 0; i < rcc.numComponents; i++)
        {
            RenderComponent *rc = rcc.components + i;
			new (rc) RenderComponent(light, &(m->submeshes[i]));
        }

		PointLightComponent *plc = addPointLightComponent(light);
		plc->intensity = Vec3(1, 1, 1);

		rm.initMaterial("", "bulbMaterial", true);
		rm.initShader("shader/light.vert", "shader/light.frag", true);

		bulb->submeshes[1].material = rm.getMaterial("", "bulbMaterial");
		bulb->submeshes[1].material->receiveLight = false;
		bulb->submeshes[1].material->shader = rm.getShader("shader/light.vert", "shader/light.frag");
		clearUniforms(bulb->submeshes[1].material);
		bulb->submeshes[1].material->vec3Uniforms.emplace("lightColor", Vec3(1, 1, 1));
	}

	// Set up directional light
	{
		Entity dirLight = makeEntity(result.ecs, "directional light");
		DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

		dlc->intensity = Vec3(.25, .25, .25);
		dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
	}

	//
	// Set up cubemap
	//
	Cubemap* cm = rm.initCubemap("cubemap/watersky", ".jpg", true);
	addCubemap(&result, cm);

	//
	// Set up hex meshes
	//
	const uint32 hexCount = 2;
	Vec3 hexPositions[hexCount];
	hexPositions[0] = Vec3(-18, -12, -40);
	hexPositions[1] = Vec3(-18, -12, -18);

	for (uint32 i = 0; i < hexCount; i++)
	{
		Entity e = makeEntity(result.ecs, "hex");
		TransformComponent *tc = addTransformComponent(e);
		tc->position = hexPositions[i];
		
		ComponentGroup<RenderComponent> rcc = addRenderComponents(e, hexMesh->submeshes.size());

		for(uint32 j = 0; j < hexMesh->submeshes.size(); j++)
		{
			RenderComponent *rc = rcc.components + j;
			new (rc) RenderComponent(e, &(hexMesh->submeshes[j]));
		}
	}

	return result;
}

void updateCameraXfm(TransformComponent* xfm)
{
	Plane movementPlane(Vec3(0, 0, 0), Vec3(0, 1, 0));
	
	real32 cameraTurnSpeed = 1.5; // Deg / pixel / Sec
	real32 cameraSpeed = 5;
	real32 deltaTS = deltaTMs / 1000.0f;

	if (keys[GLFW_KEY_LEFT_SHIFT])
	{
		cameraSpeed *= 2;
	}

	Vec3 moveRight   = normalize( project(xfm->right(), movementPlane) );
	Vec3 moveLeft    = normalize( -moveRight );
	Vec3 moveForward = normalize( project(xfm->forward(), movementPlane) );
	Vec3 moveBack    = normalize( -moveForward );

	// Uncomment this (and the asserts) to follow the pitch of the camera when
	// moving forward or backward.
	// moveForward = normalize(xfm->forward());

	assert(FLOAT_EQ(moveRight.y, 0, EPSILON));
	assert(FLOAT_EQ(moveLeft.y, 0, EPSILON));
	assert(FLOAT_EQ(moveForward.y, 0, EPSILON));
	assert(FLOAT_EQ(moveBack.y, 0, EPSILON));
	
	if (mouseXPrev != FLT_MAX && mouseYPrev != FLT_MAX)
	{
		// Rotate
		real32 deltaMouseX = mouseX - mouseXPrev;
		real32 deltaMouseY = mouseY - mouseYPrev;

		Quaternion deltaYawAndPitch;
		deltaYawAndPitch = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * -deltaMouseX * deltaTS); // yaw
		deltaYawAndPitch = deltaYawAndPitch * axisAngle(moveRight, cameraTurnSpeed * -deltaMouseY * deltaTS); // pitch


		xfm->orientation = deltaYawAndPitch * xfm->orientation;

		float camRightY = xfm->right().y;
		assert(FLOAT_EQ(camRightY, 0, EPSILON));
	}

	if (keys[GLFW_KEY_W])
	{
		xfm->position += moveForward * cameraSpeed * deltaTS;
	}
	else if (keys[GLFW_KEY_S])
	{
		xfm->position += moveBack * cameraSpeed * deltaTS;
	}
		
	if (keys[GLFW_KEY_A])
	{
		xfm->position += moveLeft * cameraSpeed * deltaTS;
	}
	else if (keys[GLFW_KEY_D])
	{
		xfm->position += moveRight * cameraSpeed * deltaTS;
	}
}


int WinMain()
{
	// INIT WINDOW
	uint32 windowWidth = 1024;
	uint32 windowHeight = 720;
	
	Window window;

	if(!initGlfwWindow(&window, windowWidth, windowHeight))
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	ResourceManager::instance().init();
	DebugDraw::instance().init();
	real32 lastTimeMs = 0;

	Scene testScene1 = buildTestScene1();
	
	// Set up camera
	Entity camera = makeEntity(testScene1.ecs, "camera");
	TransformComponent* cameraXfm = addTransformComponent(camera);
	CameraComponent* cameraComponent = addCameraComponent(camera);
	cameraComponent->projectionMatrix.perspectiveInPlace(60.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	cameraComponent->isOrthographic = false;


	//
	// Set up portal
	//
	{
		Entity portal = makeEntity(testScene1.ecs, "portal");
		
		PortalComponent* pc = addPortalComponent(portal);
		setDimensions(pc, Vec2(2, 3));
		pc->sourceScene = &testScene1;
		pc->destScene = &testScene1;

		pc->sourceSceneXfm.position = Vec3(0, 0, -5);
		pc->sourceSceneXfm.orientation = axisAngle(Vec3(0, 1, 0), 180);

		pc->destSceneXfm.position = Vec3(-18, -12, -25);
		pc->destSceneXfm.orientation = axisAngle(Vec3(0, 1, 0), 20);
	}
	
	while(!glfwWindowShouldClose(window.glfwWindow))
	{
		glfwPollEvents();
		glClearColor(.5f, 0.5f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		real32 timeS = glfwGetTime();
		real32 timeMs = timeS * 1000.0f;
		deltaTMs = timeMs - lastTimeMs;
		lastTimeMs = timeMs;

		updateCameraXfm(cameraXfm);

		if (keys[GLFW_KEY_1] && !lastKeys[GLFW_KEY_1])
		{
			debug_hidePortalContents = !debug_hidePortalContents;
		}

		renderScene(&testScene1, cameraComponent, cameraXfm);

		glfwSwapBuffers(window.glfwWindow);

		mouseXPrev = mouseX;
		mouseYPrev = mouseY;

		updateLastKeys();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	glfwTerminate();
	return 0;
}
