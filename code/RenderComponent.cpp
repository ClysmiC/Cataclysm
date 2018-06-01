#include "RenderComponent.h"
#include "GL/glew.h"

void RenderComponent::init(const Submesh &submesh)
{
    submeshOpenGlInfo = submesh.openGlInfo;
    material = submesh.material;
}

void RenderComponent::draw(TransformComponent *xfm, CameraComponent* camera, TransformComponent *cameraXfm)
{
	Mat4 m2w = xfm->modelToWorld();
	Mat4 w2v = cameraXfm->worldToView();
	
    material->bind();
    material->shader->setMat4("model", m2w);
    material->shader->setMat4("view", w2v);
    material->shader->setMat4("projection", camera->projectionMatrix);

    glBindVertexArray(submeshOpenGlInfo.VAO);
    glDrawElements(GL_TRIANGLES, submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
