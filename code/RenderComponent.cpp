#include "RenderComponent.h"
#include "GL/glew.h"

RenderComponent::RenderComponent(Entity entity, Submesh* submesh)
	: Component(entity)
{
    submeshOpenGlInfo = submesh->openGlInfo;
    material = submesh->material;
}

void drawRenderComponent(RenderComponent* renderComponent, Transform *xfm, CameraComponent* camera, Transform *cameraXfm)
{
	Mat4 m2w = modelToWorld(xfm);;
	Mat4 w2v = worldToView(cameraXfm);
	
    bind(renderComponent->material);
    setMat4(renderComponent->material->shader, "model", m2w);
    setMat4(renderComponent->material->shader, "view", w2v);
    setMat4(renderComponent->material->shader, "projection", camera->projectionMatrix);

    glBindVertexArray(renderComponent->submeshOpenGlInfo.vao);
    glDrawElements(GL_TRIANGLES, renderComponent->submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
