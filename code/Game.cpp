#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <thread>

#include "Ecs.h"
#include "ResourceManager.h"
#include "DebugDraw.h"
#include "Mesh.h"
#include "GL/glew.h"

bool keys[1024];
bool lastKeys[1024];

float mouseX;
float mouseY;
float mouseXPrev;
float mouseYPrev;

real32 deltaTMs;


void updateCamera(CameraComponent* camera, TransformComponent* cameraXfm)
{
	Plane movementPlane(Vec3(0, 0, 0), Vec3(0, 1, 0));
	
	real32 cameraTurnSpeed = 30; // Deg / Sec
	real32 cameraSpeed = 1;
	real32 deltaTS = deltaTMs / 1000.0f;

	if (mouseXPrev != FLT_MAX && mouseYPrev != FLT_MAX)
	{
		// Rotate
		real32 deltaMouseX = mouseX - mouseXPrev;
		real32 deltaMouseY = mouseY - mouseYPrev;

		Quaternion deltaYawAndPitch;
		deltaYawAndPitch = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * -deltaMouseX * deltaTS); // yaw
		deltaYawAndPitch = deltaYawAndPitch * axisAngle(Vec3(1, 0, 0), cameraTurnSpeed * deltaMouseY * deltaTS); // pitch

		cameraXfm->setOrientation(cameraXfm->orientation() * deltaYawAndPitch);
	}

	// Translate
	Vec3 position = cameraXfm->position();

	Vec3 moveRight   = normalize( project(camera->right(), movementPlane) );
	Vec3 moveLeft    = normalize( -moveRight );
	Vec3 moveForward = normalize( project(camera->forward(), movementPlane) );
	Vec3 moveBack    = normalize( -moveForward );
	
	if (keys[GLFW_KEY_W])
	{
		position += moveForward * cameraSpeed * deltaTS;
	}
	else if (keys[GLFW_KEY_S])
	{
		position += moveBack * cameraSpeed * deltaTS;
	}
		
	if (keys[GLFW_KEY_A])
	{
		position += moveLeft * cameraSpeed * deltaTS;
	}
	else if (keys[GLFW_KEY_D])
	{
		position += moveRight * cameraSpeed * deltaTS;
	}

	cameraXfm->setPosition(position);
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
	
	Mesh *mesh = ResourceManager::instance().initMesh("cyborg/cyborg.obj", true, true);
	Mesh *bulb = ResourceManager::instance().initMesh("bulb/bulb.obj", false, true);
	
	real32 lastTimeMs = 0;
	
    Ecs ecs;
	
	// Set up camera
	Entity camera = ecs.makeEntity();
	TransformComponent* cameraXfm = ecs.addTransformComponent(camera);
	CameraComponent* cameraComponent = ecs.addCameraComponent(camera);
	cameraComponent->projectionMatrix.perspectiveInPlace(60.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	cameraComponent->isOrthographic = false;

	DebugDraw::instance().init(camera);
	
    Entity test = ecs.makeEntity();
    // Set up test mesh
    {
        TransformComponent *tc = ecs.addTransformComponent(test);
        tc->setPosition(0, -2, -5);
		// tc->setScale(0.25);
		
        ComponentGroup<RenderComponent> rcc = ecs.addRenderComponents(test, mesh->submeshes.size());

        for(uint32 i = 0; i < rcc.numComponents; i++)
        {
            RenderComponent *rc = rcc.components + i;
            rc->init(mesh->submeshes[i]);
        }
    }
	
	Entity light = ecs.makeEntity();
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

	//
	//
	// Set up cubemap
	uint32 skyboxVao;
	uint32 skyboxVbo;
	{
		ResourceManager& rm = ResourceManager::instance();
		rm.initShader("shader/cubemap.vert", "shader/cubemap.frag", true);
		rm.initCubemap("cubemap/watersky", ".jpg", true);

		real32 vertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};
		
		glGenBuffers(1, &skyboxVbo);
		glGenVertexArrays(1, &skyboxVao);
		glBindVertexArray(skyboxVao);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
	}
	//
	//
	//

	auto v = glGetError();

	TransformComponent *lightXfm = ecs.getTransformComponent(light);
    TransformComponent *testXfm = ecs.getTransformComponent(test);

	Cubemap* cm = ResourceManager::instance().getCubemap("cubemap/watersky");
	Shader* cubemapShader = ResourceManager::instance().getShader("shader/cubemap.vert", "shader/cubemap.frag");
	
	while(!glfwWindowShouldClose(window.glfwWindow))
	{
		v = glGetError();
		
		glfwPollEvents();
		glClearColor(.5f, 0.5f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		v = glGetError();

		//
		// Draw skybox
		{
			// TODO: should this be part of the camera?
			Mat4 projection;
			projection.perspectiveInPlace(60.0f, 4.0f / 3.0f, 0.1f, 1000.0f);

			Mat4 view = cameraComponent->worldToViewMatrix();
			Mat4 viewProjectionSansTranslation = projection * view.mat3ifyInPlace();

			cubemapShader->bind();
			
			glActiveTexture(GL_TEXTURE0);
			cubemapShader->setInt("cubemap", 0);
			cubemapShader->setMat4("viewProjectionSansTranslation", viewProjectionSansTranslation);

			glDepthMask(GL_FALSE);
			glBindVertexArray(skyboxVao);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cm->openGlHandle);
			
			glDrawArrays(GL_TRIANGLES, 0, 36);
			
			glDepthMask(GL_TRUE);
		}
		//
		//


		v = glGetError();

		real32 timeS = glfwGetTime();
		real32 timeMs = timeS * 1000.0f;
		deltaTMs = timeMs - lastTimeMs;
		lastTimeMs = timeMs;

		lightXfm->setPosition(Vec3(2 * sinf(timeS), 0, -2));
		updateCamera(cameraComponent, cameraXfm);
		
		// Quaternion lightRot = axisAngle(Vec3(0, 1, 0), 30 * timeS);
		Quaternion meshRot = axisAngle(Vec3(0, 1, 0), 30 * timeS); 
        //testXfm->setOrientation(meshRot);

        ecs.renderAllRenderComponents(cameraComponent);
		DebugDraw::instance().drawAARect3(Vec3(2, 0, -6), Vec3(2, 3, 1));

		glfwSwapBuffers(window.glfwWindow);

		mouseXPrev = mouseX;
		mouseYPrev = mouseY;
		
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	glfwTerminate();
	return 0;
}
