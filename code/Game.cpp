#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <thread>

#include "Ecs.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "GL/glew.h"

bool keys[1024];
bool lastKeys[1024];
real32 deltaTMs;


void updateCamera(TransformComponent* cameraXfm)
{
	real32 cameraTurnSpeed = 30; // Deg / Sec
	real32 cameraSpeed = 1;
	real32 deltaTS = deltaTMs / 1000.0f;

	Vec3 position = cameraXfm->position();
	Quaternion orientation = cameraXfm->orientation();
	
	if (keys[GLFW_KEY_W])
	{
		position += cameraForward(cameraXfm) * cameraSpeed * deltaTS;
	}
	else if (keys[GLFW_KEY_S])
	{
		position += cameraBack(cameraXfm) * cameraSpeed * deltaTS;
	}
		
	if (keys[GLFW_KEY_A])
	{
		position += cameraLeft(cameraXfm) * cameraSpeed * deltaTS;
	}
	else if (keys[GLFW_KEY_D])
	{
		position += cameraRight(cameraXfm) * cameraSpeed * deltaTS;
	}

	if (keys[GLFW_KEY_Q])
	{
		Quaternion turn = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * deltaTS);
		orientation = orientation * turn;
	}
	else if (keys[GLFW_KEY_E])
	{
		Quaternion turn = axisAngle(Vec3(0, 1, 0), -cameraTurnSpeed * deltaTS);
		orientation = orientation * turn;
	}

	cameraXfm->setPosition(position);
	cameraXfm->setOrientation(orientation);
}


int WinMain()
{
	// INIT WINDOW
	uint32 windowWidth = 1024;
	uint32 windowHeight = 720;
	
	Window window;

	if(window.init(windowWidth, windowHeight) < 0)
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

	ResourceManager::instance().init();
	Mesh *mesh = ResourceManager::instance().initMesh("nanosuit/nanosuit.obj", true, true);
	Mesh *bulb = ResourceManager::instance().initMesh("bulb/bulb.obj", false, true);
	
	real32 lastTimeMs = 0;
	
    Ecs ecs;
	
	// Set up camera
	Entity camera = ecs.nextEntityId();
	TransformComponent* cameraXfm = ecs.addTransformComponent(camera);
	
    Entity test = ecs.nextEntityId();
    // Set up test mesh
    {
        TransformComponent *tc = ecs.addTransformComponent(test);
        tc->setPosition(0, -2, -4);
		tc->setScale(0.25);
		
        ComponentGroup<RenderComponent> rcc = ecs.addRenderComponents(test, mesh->submeshes.size());

        for(uint32 i = 0; i < rcc.numComponents; i++)
        {
            RenderComponent *rc = rcc.components + i;
            rc->init(mesh->submeshes[i]);
        }
    }
	
	Entity light = ecs.nextEntityId();
	// Set up light
	{
		ResourceManager& rm = ResourceManager::instance();
		
        Mesh* m = bulb;
        
        TransformComponent *tc = ecs.addTransformComponent(light);
        tc->setPosition(0, 0, -2);
        tc->setScale(.35);
		
        ComponentGroup<RenderComponent> rcc = ecs.addRenderComponents(light, m->submeshes.size());

        for(uint32 i = 0; i < rcc.numComponents; i++)
        {
            RenderComponent *rc = rcc.components + i;
            rc->init(m->submeshes[i]);
        }

		PointLightComponent *plc = ecs.addPointLightComponent(light);
		plc->intensity = Vec3(5, 5, 5);

		rm.initMaterial("", "bulbMaterial", true);
		rm.initShader("shader/light.vert", "shader/light.frag", true);

		bulb->submeshes[1].material = rm.getMaterial("", "bulbMaterial");
		bulb->submeshes[1].material->receiveLight = false;
		bulb->submeshes[1].material->shader = rm.getShader("shader/light.vert", "shader/light.frag");
		bulb->submeshes[1].material->clearUniforms();
		bulb->submeshes[1].material->vec3Uniforms.emplace("lightColor", Vec3(1, 1, 1));
	}

	TransformComponent *lightXfm = ecs.getTransformComponent(light);
    TransformComponent *testXfm = ecs.getTransformComponent(test);

	while(!glfwWindowShouldClose(window.glfwWindow))
	{
		glfwPollEvents();
		glClearColor(.5f, 0.5f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		auto v = glGetError();

		real32 timeS = glfwGetTime();
		real32 timeMs = timeS * 1000.0f;
		deltaTMs = timeMs - lastTimeMs;
		lastTimeMs = timeMs;

		lightXfm->setPosition(Vec3(2 * sinf(timeS), 0, -2));
		updateCamera(cameraXfm);
		
		// Quaternion lightRot = axisAngle(Vec3(0, 1, 0), 30 * timeS);
		Quaternion meshRot = axisAngle(Vec3(0, 1, 0), 30 * timeS); 
        //testXfm->setOrientation(meshRot);

        ecs.renderAllRenderComponents(cameraXfm);

		glfwSwapBuffers(window.glfwWindow);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	glfwTerminate();
	return 0;
}
