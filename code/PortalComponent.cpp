#include "GL/glew.h"

#include "PortalComponent.h"
#include "ResourceManager.h"

PortalComponent::PortalComponent() {}

uint32 PortalComponent::quadVbo_ = 0;
uint32 PortalComponent::quadVao_ = 0;
real32 PortalComponent::quadVertices[] = {
	// positions          
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,

	-0.5f, -0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f
};

Shader*
PortalComponent::shader()
{
	static Shader* shader_ = nullptr;

	if (shader_ == nullptr)
	{
		shader_ = ResourceManager::instance().initShader("shader/portal.vert", "shared/portal.frag", true);
	}

	return shader_;
}

uint32
PortalComponent::quadVbo()
{
	if (quadVbo_ == 0 || quadVao_ == 0)
	{
		initQuadVboAndVao();
	}

	return quadVbo_;
}

uint32
PortalComponent::quadVao()
{
	if (quadVbo_ == 0 || quadVao_ == 0)
	{
		initQuadVboAndVao();
	}

	return quadVao_;
}

void
PortalComponent::initQuadVboAndVao()
{
	glGenBuffers(1, &quadVbo_);
	glGenVertexArrays(1, &quadVao_);
	glBindVertexArray(quadVao_);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(real32), (void*)0);
}
