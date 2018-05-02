#include "Entity.h"
#include "Mesh.h"
#include "Camera.h"

Entity::Entity()
    :
	scale(1, 1, 1),
	orientation(0, 0, 0, 1)
{
}


Entity::~Entity()
{
}

void Entity::draw(Camera camera)
{
    if (mesh != nullptr)
    {
        Mat4 xfm = transform();
        mesh->draw(xfm, camera);
    }
}
