#include "TransformComponent.h"
#include "Reflection.h"

TransformComponent::TransformComponent()
    : TransformComponent(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position)
    : TransformComponent(position, Quaternion(), Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation)
    : TransformComponent(position, orientation, Vec3(1, 1, 1))
{
}

TransformComponent::TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale)
    : Transform(position, orientation, scale)
{
}

void reflectTransformComponent(IReflector* reflector, uint32 startingOffset)
{
    reflector->pushStruct("Position");
    reflectVec3(reflector, startingOffset + offsetof(TransformComponent, position));
    reflector->popStruct();

    reflector->pushStruct("Orientation");
    reflectQuaternion(reflector, startingOffset + offsetof(TransformComponent, orientation));
    reflector->popStruct();

    reflector->pushStruct("Scale");
    reflectVec3(reflector, startingOffset + offsetof(TransformComponent, scale));
    reflector->popStruct();
}
