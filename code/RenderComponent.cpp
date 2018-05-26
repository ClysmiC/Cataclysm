#include "RenderComponent.h"
#include "GL/glew.h"

void RenderComponent::init(const Submesh &submesh)
{
    submeshOpenGlInfo = submesh.openGlInfo;
    material = submesh.material;
}

void RenderComponent::draw(TransformComponent *xfm, CameraComponent *camera)
{
	Mat4 view = camera->worldToViewMatrix();

	Mat4 transform = xfm->matrix();
	
    material->bind();
    material->shader->setMat4("model", transform);
    material->shader->setMat4("view", view);
    material->shader->setMat4("projection", camera->projectionMatrix);

    glBindVertexArray(submeshOpenGlInfo.VAO);
    glDrawElements(GL_TRIANGLES, submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
