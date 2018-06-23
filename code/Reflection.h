#pragma once

#include <string>
#include "als_math.h"

enum ReflectionPurpose
{
    OTHER,
    UI,
    SERIALIZATION
};

#define ReflectionMaxStructDepth 16
struct IReflector
{
    void* reflectionTarget;
    ReflectionPurpose purpose = OTHER;

    // If you need to recursively reflect a struct (i.e., a TransformComponent reflects a Vec3 struct),
    // do the following:
    // reflector->pushStruct("position", offsetof(TransformComponent, position));
    // reflect(&xfm->position, reflector)
    // reflector->popStruct();
    virtual void pushStruct(std::string name) = 0;
    virtual void popStruct() = 0;
    
    virtual void startReflection() = 0;
    virtual void consumeInt32(std::string name, uint32 offset) = 0;
    virtual void consumeUInt32(std::string name, uint32 offset) = 0;
    virtual void consumeFloat32(std::string name, uint32 offset) = 0;
    virtual void consumeFloat64(std::string name, uint32 offset) = 0;
    virtual void consumeBool(std::string name, uint32 offset) = 0;
    virtual void consumeEnum(std::string name, uint32 offset) = 0; // todo: pass some table to look up a friendly name for the value
    virtual void endReflection() = 0;
};

struct UiReflector : public IReflector
{
    uint32 tabLevel = 0;
    
    void pushStruct(std::string name) override;
    void popStruct() override;

    void startReflection() override {};
    void consumeInt32(std::string name, uint32 offset) override {};
    void consumeUInt32(std::string name, uint32 offset) override {};
    void consumeFloat32(std::string name, uint32 offset) override;
    void consumeFloat64(std::string name, uint32 offset) override {};
    void consumeBool(std::string name, uint32 offset) override {};
    void consumeEnum(std::string name, uint32 offset) override {}; // todo: pass some table to look up a friendly name for the value
    void endReflection() override {};
};

#include "Entity.h"
void testUiReflection(Entity e);

void reflectVec2(IReflector* reflector, uint32 startingOffset);
void reflectVec3(IReflector* reflector, uint32 startingOffset);
void reflectVec4(IReflector* reflector, uint32 startingOffset);
void reflectQuaternion(IReflector* reflector, uint32 startingOffset);
