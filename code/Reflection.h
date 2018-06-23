#pragma once

#include <string>
#include "als_math.h"

struct ColliderComponent;
struct CameraComponent;

enum ReflectionPurpose
{
    OTHER,
    UI,
    SERIALIZATION
};

struct IReflector
{
    IReflector() = default;
    IReflector(ReflectionPurpose purpose);
    
    void* reflectionTarget;
    ReflectionPurpose purpose = OTHER;
    
    virtual void pushStruct(std::string name) = 0;
    virtual void popStruct() = 0;
    
    virtual void startReflection(std::string label) = 0;
    virtual void consumeInt32(std::string name, uint32 offset) = 0;
    virtual void consumeUInt32(std::string name, uint32 offset) = 0;
    virtual void consumeFloat32(std::string name, uint32 offset) = 0;
    virtual void consumeFloat64(std::string name, uint32 offset) = 0;
    virtual void consumeBool(std::string name, uint32 offset) = 0;
    virtual void consumeEnum(std::string name, uint32 offset) = 0; // todo: pass some table to look up a friendly name for the value??
    virtual void endReflection() = 0;
};

inline std::string tabString(uint32 indentationLevel)
{
    std::string result;
    for (uint32 i = 0; i < indentationLevel; i++)
    {
        result += "\t";
    }

    return result;
}

struct UiReflector : public IReflector
{
    UiReflector();
    
    uint32 indentationLevel = 0;
    
    void pushStruct(std::string name) override;
    void popStruct() override;

    void startReflection(std::string label) override;
    void consumeInt32(std::string name, uint32 offset) override;
    void consumeUInt32(std::string name, uint32 offset) override;
    void consumeFloat32(std::string name, uint32 offset) override;
    void consumeFloat64(std::string name, uint32 offset) override;
    void consumeBool(std::string name, uint32 offset) override;
    void consumeEnum(std::string name, uint32 offset) override; // todo: pass some table to look up a friendly name for the value?? UI can enumerate all values in table into a dropdown?
    void endReflection() override;
};

#include "Entity.h"
void testUiReflection(Entity e);

void reflectVec2(IReflector* reflector, uint32 startingOffset);
void reflectVec3(IReflector* reflector, uint32 startingOffset);
void reflectVec4(IReflector* reflector, uint32 startingOffset);
void reflectQuaternion(IReflector* reflector, uint32 startingOffset);

void reflectTransformComponent(IReflector* reflector, uint32 startingOffset);
void reflectColliderComponent(IReflector* reflector, ColliderComponent* collider, uint32 startingOffset);
void reflectCameraComponent(IReflector* reflector, CameraComponent* camera, uint32 startingOffset);
void reflectDirectionalLightComponent(IReflector* reflector, uint32 startingOffset);
void reflectPointLightComponent(IReflector* reflector, uint32 startingOffset);
void reflectPortalComponent(IReflector* reflector, uint32 startingOffset);
void reflectRenderComponent(IReflector* reflector, uint32 startingOffset);
