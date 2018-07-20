#pragma once

#include "als_math.h"
#include <vector>

struct Transform;
struct LiteTransform : public LiteTransform
{
    LiteTransform();
    LiteTransform(Vec3 position);
    LiteTransform(Vec3 position, Quaternion orientation);
    LiteTransform(Vec3 position, Quaternion orientation, Vec3 scale);
    LiteTransform(Transform& transform);

    virtual inline Vec3 position() { return this->_position; }
    virtual inline Quaternion orientation() { return this->_orientation; }
    virtual inline Vec3 scale() { return this->_scale; }
    virtual inline void setPosition(Vec3 position) { this->_position = position; }
    virtual inline void setOrientation(Quaternion orientation) { this->_orientation = orientation; }
    virtual inline void setScale(Vec3 scale) { this->_scale = scale; }

private:
    Vec3 _position;
    Quaternion _orientation;
    Vec3 _scale;
};

struct Transform
{
    Transform();
    Transform(Vec3 position);
    Transform(Vec3 position, Quaternion orientation);
    Transform(Vec3 position, Quaternion orientation, Vec3 scale);
    Transform(const LiteTransform& transform);

    virtual Transform* getParent() = 0;
    virtual std::vector<Transform*> getChildren() = 0;
    
    inline Vec3 position() override
    {
        if (dirty) recalculateWorld();
        return worldPosition;
    }

    inline Quaternion orientation() override
    {
        if (dirty) recalculateWorld();
        return worldOrientation;
    }

    inline Vec3 scale() override
    {
        if (dirty) recalculateWorld();
        return worldScale;
    }

    inline Vec3 localPosition()
    {
        return _localPosition;
    }

    inline Quaternion localOrientation()
    {
        return _localOrientation;
    }

    inline Vec3 localScale()
    {
        return _localScale;
    }

    inline Vec3 left()
    {
        Vec3 result = this->orientation() * Vec3(-1, 0, 0);
        return result;
    }

    inline Vec3 right()
    {
        Vec3 result = this->orientation() * Vec3(1, 0, 0);
        return result;
    }

    inline Vec3 up()
    {
        Vec3 result = this->orientation() * Vec3(0, 1, 0);
        return result;
    }

    inline Vec3 down()
    {
        Vec3 result = this->orientation() * Vec3(0, -1, 0);
        return result;
    }

    inline Vec3 forward()
    {
        Vec3 result = this->orientation() * Vec3(0, 0, -1);
        return result;
    }

    inline Vec3 back()
    {
        Vec3 result = this->orientation() * Vec3(0, 0, 1);
        return result;
    }

    Mat4 matrix();

    void setLocalPosition(Vec3 position);
    void setLocalOrientation(Quaternion orientation);
    void setLocalScale(Vec3 scale);
    void setPosition(Vec3 position) override;
    void setOrientation(Quaternion orientation) override;
    void setScale(Vec3 scale) override;

    void markSelfAndChildrenDirty();
    void recalculateWorld();
    
protected:
    //
    // Authoratative
    //
    Vec3 _localPosition;
    Quaternion _localOrientation;
    Vec3 _localScale;

    //
    // Cached
    //
    bool dirty = true;
    Vec3 worldPosition;
    Quaternion worldOrientation;
    Vec3 worldScale;

    Mat4 toParent;
    Mat4 toWorld;
};



//
// Returns the transform aToC in the out parameters
//
void multiplyTransforms(
    Vec3 aToBPos, Quaternion aToBOrientation, Vec3 aToBScale,
    Vec3 bToCPos, Quaternion bToCOrientation, Vec3 bToCScale,
    Vec3* outPosition, Quaternion* outOrientation, Vec3* outScale
);

// TODO: put this in camera component code?
Mat4 worldToView(Transform* cameraXfm);
