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
    
    virtual bool pushStruct(std::string name) = 0;
    virtual void popStruct() = 0;
    
    virtual bool startReflection(std::string label) = 0;
    virtual void endReflection() = 0;

    // Reflectors may modify the existing value, so the return value
    // will be the old existing value if unchanged, or the new value if
    // it is modified.
    virtual int32   consumeInt32(std::string name, uint32 offset) = 0;
    virtual uint32  consumeUInt32(std::string name, uint32 offset) = 0;
    virtual float32 consumeFloat32(std::string name, uint32 offset) = 0;
    virtual float64 consumeFloat64(std::string name, uint32 offset) = 0;
    virtual bool    consumeBool(std::string name, uint32 offset) = 0;

    // Note: Consumable enums must have underlying type uint32
    // Potential todo: make consumeEnumInt32, consumeEnumUInt32, etc. (but I think it is fine to assume uint32 and enforce
    // that for now)
    virtual uint32  consumeEnum(std::string name, uint32 offset, std::string* enumNames, uint32 enumValueCount) = 0;
    
};

struct UiReflector : public IReflector
{
    UiReflector();
    
    uint32 indentationLevel = 0;
    
    bool pushStruct(std::string name) override;
    void popStruct() override;

    bool startReflection(std::string label) override;
    void endReflection() override;
    
    int32   consumeInt32(std::string name, uint32 offset) override;
    uint32  consumeUInt32(std::string name, uint32 offset) override;
    float32 consumeFloat32(std::string name, uint32 offset) override;
    float64 consumeFloat64(std::string name, uint32 offset) override;
    bool    consumeBool(std::string name, uint32 offset) override;
    uint32  consumeEnum(std::string name, uint32 offset, std::string* enumNames, uint32 enumValueCount) override;
    
};

#include "Entity.h"
bool testUiReflection(Entity e);

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
