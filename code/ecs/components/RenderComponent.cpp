#include "Transform.h"
#include "RenderComponent.h"
#include "Camera.h"

#include "GL/glew.h"

#include "ecs/Ecs.h"
#include "Aabb.h"
#include "DebugDraw.h"
#include "resource/resources/Mesh.h"
#include "resource/resources/Shader.h"

RenderComponent::RenderComponent(Entity entity, Submesh* submesh_)
    : Component(entity)
{
    submesh = submesh_;
    submeshOpenGlInfo = submesh->openGlInfo;
    material = submesh->material;
}

Mesh* getMesh(Entity e)
{
    RenderComponent* r = getRenderComponent(e);

    if (!r) return nullptr;

    return r->submesh->mesh;
}

void drawRenderComponent(RenderComponent* renderComponent, ITransform *xfm, ICamera* camera, ITransform *cameraXfm, uint32 shadowMapTextureId, Mat4& lightMatrix)
{
    Mat4 m2w = xfm->matrix();;
    Mat4 w2v = worldToView(cameraXfm);

    auto v = glGetError();
    bind(renderComponent->material);

    v = glGetError();

    setMat4(renderComponent->material->shader, "model", m2w);

    v = glGetError();

    setMat4(renderComponent->material->shader, "view", w2v);

    v = glGetError();

    setMat4(renderComponent->material->shader, "projection", camera->projectionMatrix);

    v = glGetError();

	setMat4(renderComponent->material->shader, "lightMatrix", lightMatrix);

    v = glGetError();

	bindShadowMap(renderComponent->material, shadowMapTextureId);

    v = glGetError();
    
    glBindVertexArray(renderComponent->submeshOpenGlInfo.vao);
    glDrawElements(GL_TRIANGLES, renderComponent->submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    v = glGetError();
    assert(v == 0);
}

void drawRenderComponentWithShader(RenderComponent* renderComponent, Shader* shader, ITransform *xfm, ICamera* camera, ITransform* cameraXfm)
{
    // @Cut-n-paste from drawRenderComponent
    
    Mat4 m2w = xfm->matrix();;
    Mat4 w2v = worldToView(cameraXfm);

    bind(shader);
    setMat4(shader, "model", m2w);
    setMat4(shader, "view", w2v);
    setMat4(shader, "projection", camera->projectionMatrix);
    
    glBindVertexArray(renderComponent->submeshOpenGlInfo.vao);
    glDrawElements(GL_TRIANGLES, renderComponent->submeshOpenGlInfo.indicesSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
