#include "RenderComponent.h"
#include "GL/glew.h"

void RenderComponent::RenderComponent(Submesh* submesh)
{
    submeshOpenGlInfo = submesh->openGlInfo;
    material = submesh->material;
}

void drawRenderComponent(RenderComponent* renderComponent, TransformComponent *xfm, CameraComponent* camera, TransformComponent *cameraXfm)
{
	Mat4 m2w = xfm->modelToWorld();
	Mat4 w2v = cameraXfm->worldToView();
	
    renderComponent->material->bind();
    renderComponent->material->shader->setMat4("model", m2w);
    renderComponent->material->shader->setMat4("view", w2v);
    renderComponent->material->shader->setMat4("projection", camera->projectionMatrix);

    glBindVertexArray(renderComponent->submeshOpenGlInfo.vao);
    glDrawElements(GL_TRIANGLES, submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
