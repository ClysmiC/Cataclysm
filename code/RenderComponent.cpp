#include "RenderComponent.h"
#include "GL/glew.h"

void RenderComponent::init(const Submesh &submesh)
{
    submeshOpenGlInfo = submesh.openGlInfo;
    material = submesh.material;
}

void RenderComponent::draw(TransformComponent* xfm, Camera& camera)
{
	Mat4 view = camera.worldToView();

	// TODO: should this be part of the camera?
    Mat4 projection;
    projection.perspectiveInPlace(60.0f, 4.0f / 3.0f, 0.1f, 1000.0f);

	Mat4 transform = xfm->matrix();
	
    material->bind();
    material->shader->setMat4("model", transform);
    material->shader->setMat4("view", view);
    material->shader->setMat4("projection", projection);

    glBindVertexArray(submeshOpenGlInfo.VAO);
    glDrawElements(GL_TRIANGLES, submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
