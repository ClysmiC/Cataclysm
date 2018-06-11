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

///////////////////////////////////////////////////////////////////////
//////////// BEGIN SCRATCHPAD (throwaway or refactorable code)

void updateLastKeys()
{
	for (uint32 i = 0; i < 1024; i++)
	{
		lastKeys[i] = keys[i];
	}
}

void buildTestScene1(Scene* scene)
{
	ResourceManager& rm = ResourceManager::instance();
	
	Mesh *hexMesh = rm.initMesh("hex/hex.obj", true, true);
	Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, true);

	// Set up directional light
	{
		Entity dirLight = makeEntity(&scene->ecs, "directional light");
		DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

		dlc->intensity = Vec3(.25, .25, .25);
		dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
	}

	//
	// Set up cubemap
	//
	Cubemap* cm = rm.initCubemap("cubemap/watersky", ".jpg", true);
	addCubemap(scene, cm);

	//
	// Set up hex meshes
	//
	const uint32 hexCount = 2;
	Vec3 hexPositions[hexCount];
	hexPositions[0] = Vec3(-18, -12, -40);
	hexPositions[1] = Vec3(-18, -12, -18);

	for (uint32 i = 0; i < hexCount; i++)
	{
		Entity e = makeEntity(&scene->ecs, "hex");
		TransformComponent *tc = addTransformComponent(e);
		tc->position = hexPositions[i];
		tc->scale = Vec3(.25);
		
		ComponentGroup<RenderComponent> rcc = addRenderComponents(e, hexMesh->submeshes.size());

		for(uint32 j = 0; j < hexMesh->submeshes.size(); j++)
		{
			RenderComponent *rc = rcc.components + j;
			new (rc) RenderComponent(e, &(hexMesh->submeshes[j]));
		}
	}
}

void buildTestScene2(Scene* scene)
{
	ResourceManager& rm = ResourceManager::instance();
	
	Mesh *icosahedronMesh = rm.initMesh("icosahedron/icosahedron.obj", true, true);

	// Set up directional light
	{
		Entity dirLight = makeEntity(&scene->ecs, "directional light");
		DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

		dlc->intensity = Vec3(.25, .25, .25);
		dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
	}

	//
	// Set up cubemap
	//
	Cubemap* cm = rm.initCubemap("cubemap/CloudyLightRays", ".png", true);
	addCubemap(scene, cm);

	//
	// Set up icosahedron meshes
	//
	const uint32 icosahedronCount = 4;
	Vec3 icosahedronPositions[icosahedronCount];
	icosahedronPositions[0] = Vec3(-4, 2, 3);
	icosahedronPositions[1] = Vec3(-5, 4, 3);
	icosahedronPositions[2] = Vec3(-6, 0, 3);
	icosahedronPositions[3] = Vec3(-2, 2, 1);

	for (uint32 i = 0; i < icosahedronCount; i++)
	{
		Entity e = makeEntity(&scene->ecs, "icosahedron");
		TransformComponent *tc = addTransformComponent(e);
		tc->position = icosahedronPositions[i];
		
		ComponentGroup<RenderComponent> rcc = addRenderComponents(e, icosahedronMesh->submeshes.size());

		for(uint32 j = 0; j < icosahedronMesh->submeshes.size(); j++)
		{
			RenderComponent *rc = rcc.components + j;
			new (rc) RenderComponent(e, &(icosahedronMesh->submeshes[j]));
		}
	}
}

void buildTestScene3(Scene* scene)
{
	ResourceManager& rm = ResourceManager::instance();
	
	Mesh *shuttleMesh = rm.initMesh("shuttle/shuttle.obj", true, true);
	Mesh *bulb = rm.initMesh("bulb/bulb.obj", false, true);

	// Set up directional light
	{
		Entity dirLight = makeEntity(&scene->ecs, "directional light");
		DirectionalLightComponent* dlc = addDirectionalLightComponent(dirLight);

		dlc->intensity = Vec3(.25, .25, .25);
		dlc->direction = Vec3(-.2, -1, -1).normalizeInPlace();
	}

	//
	// Set up cubemap
	//
	Cubemap* cm = rm.initCubemap("cubemap/SunSet", ".png", true);
	addCubemap(scene, cm);

	//
	// Set up shuttle meshes
	//
	const uint32 shuttleCount = 1;
	Vec3 shuttlePositions[shuttleCount];
	shuttlePositions[0] = Vec3(0, 0, -10);

	for (uint32 i = 0; i < shuttleCount; i++)
	{
		Entity e = makeEntity(&scene->ecs, "shuttle");
		TransformComponent *tc = addTransformComponent(e);
		tc->position = shuttlePositions[i];
		tc->scale = Vec3(.25);
		
		ComponentGroup<RenderComponent> rcc = addRenderComponents(e, shuttleMesh->submeshes.size());

		for(uint32 j = 0; j < shuttleMesh->submeshes.size(); j++)
		{
			RenderComponent *rc = rcc.components + j;
			new (rc) RenderComponent(e, &(shuttleMesh->submeshes[j]));
		}
	}
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

void updateGame(Game* game)
{
	assert(game->activeScene != nullptr);
	CameraComponent* camComponent = getCameraComponent(game->activeCamera);
	TransformComponent* camXfm = getTransformComponent(game->activeCamera);
	renderScene(game->activeScene, camComponent, camXfm);
}

// END SCRATCHPAD
///////////////////////////////////////////////////////////////////

Scene* makeScene(Game* game)
{
	assert(game->numScenes < MAX_SCENES);
	Scene* result = game->scenes + game->numScenes;
	
	new (result) Scene();

	result->game = game;
	game->numScenes++;

	return result;
}

void makeSceneActive(Game* game, Scene* scene)
{
	game->activeScene = scene;
}

void makeCameraActive(Game* game, Entity camera)
{
	assert(getCameraComponent(camera) != nullptr);
	assert(getTransformComponent(camera) != nullptr);

	game->activeCamera = camera;
}

std::vector<PortalComponent*> portalsInScene(Scene* scene)
{
	std::vector<PortalComponent*> result;
	
	for (uint32 i = 0; i < scene->game->globalScene.ecs.portals.size; i++)
	{
		PortalComponent* pc = &scene->game->globalScene.ecs.portals.components[i];
		
		if (pc->scene1 == scene || pc->scene2 == scene)
		{
			result.push_back(pc);
		}
	}

	return result;
}

int main()
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

	Game* game = new Game();
	
	Scene* testScene1 = makeScene(game);
	buildTestScene1(testScene1);
	
	Scene* testScene2 = makeScene(game);
	buildTestScene2(testScene2);

	Scene* testScene3 = makeScene(game);
	buildTestScene3(testScene3);
	
	// Set up camera
	Entity camera = makeEntity(&testScene1->ecs, "camera");
	TransformComponent* cameraXfm = addTransformComponent(camera);
	CameraComponent* cameraComponent = addCameraComponent(camera);
	cameraComponent->projectionMatrix.perspectiveInPlace(60.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	cameraComponent->isOrthographic = false;

	//
	// Set up portal from scene 1<->2
	//
	{
		Entity portal = makeEntity(&game->globalScene.ecs, "portal");
		
		PortalComponent* pc = addPortalComponent(portal);
		setDimensions(pc, Vec2(2, 3));
		pc->scene1 = testScene1;
		pc->scene2 = testScene2;

		pc->scene1Xfm.position = Vec3(0, 0, -10);
		pc->scene1Xfm.orientation = axisAngle(Vec3(0, 1, 0), 180);

		pc->scene2Xfm.position = Vec3(1, 2, 3);
		pc->scene2Xfm.orientation = axisAngle(Vec3(0, 1, 0), 90);
	}

	//
	// Set up portal from scene 1<->3
	//
	{
		Entity portal = makeEntity(&game->globalScene.ecs, "portal");
		
		PortalComponent* pc = addPortalComponent(portal);
		setDimensions(pc, Vec2(2, 3));
		pc->scene1 = testScene1;
		pc->scene2 = testScene3;

		pc->scene1Xfm.position = Vec3(0, 0, -10);
		pc->scene1Xfm.orientation = axisAngle(Vec3(0, 1, 0), 0);

		pc->scene2Xfm.position = Vec3(0, 0, 0);
		pc->scene2Xfm.orientation = axisAngle(Vec3(0, 1, 0), 0);
	}

	makeSceneActive(game, testScene1);
	makeCameraActive(game, camera);
	
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

		updateGame(game);

		glfwSwapBuffers(window.glfwWindow);

		mouseXPrev = mouseX;
		mouseYPrev = mouseY;

		updateLastKeys();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	glfwTerminate();
	return 0;
}
