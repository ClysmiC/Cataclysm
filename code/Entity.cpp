#include "Entity.h"

Entity::Entity()
    : scale(1, 1, 1)
{
}


Entity::~Entity()
{
}

void Entity::draw()
{
    if (mesh != nullptr)
    {
        Mat4 xfm = transform();
        mesh->draw(xfm);
    }
}
