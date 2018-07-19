#pragma once

#include "als_math.h"

struct Transform
{

    Transform();
    Transform(Vec3 position);
    Transform(Vec3 position, Quaternion orientation);
    Transform(Vec3 position, Quaternion orientation, Vec3 scale);

    inline void setPosition(Vec3 position)
    {
        this->_position = position;
        this->xfmDirty = true;
    }

    inline void setOrientation(Quaternion orientation)
    {
        this->_orientation = orientation;
        this->xfmDirty = true;
    }

    inline void setScale(Vec3 scale)
    {
        this->_scale = scale;
        this->xfmDirty = true;
    }

    inline Vec3 position()
    {
        return this->_position;
    }

    inline Quaternion orientation()
    {
        return this->_orientation;
    }

    inline Vec3 scale()
    {
        return this->_scale;
    }

    inline Vec3 left()
    {
        Vec3 result = this->_orientation * Vec3(-1, 0, 0);
        return result;
    }

    inline Vec3 right()
    {
        Vec3 result = this->_orientation * Vec3(1, 0, 0);
        return result;
    }

    inline Vec3 up()
    {
        Vec3 result = this->_orientation * Vec3(0, 1, 0);
        return result;
    }

    inline Vec3 down()
    {
        Vec3 result = this->_orientation * Vec3(0, -1, 0);
        return result;
    }

    inline Vec3 forward()
    {
        Vec3 result = this->_orientation * Vec3(0, 0, -1);
        return result;
    }

    inline Vec3 back()
    {
        Vec3 result = this->_orientation * Vec3(0, 0, 1);
        return result;
    }

    inline Mat4 matrix()
    {
        if (xfmDirty)
        {
            new (&this->xfm) Mat4; // reset to identity
            
            this->xfm.scaleInPlace(this->_scale);
            this->xfm.rotateInPlace(this->_orientation);
            this->xfm.translateInPlace(this->_position);
            this->xfmDirty = false;
        }
    
        return this->xfm;
    }
    
private:
    Vec3 _position;
    Quaternion _orientation;
    Vec3 _scale;

    bool xfmDirty = true;
    Mat4 xfm;
};

//
// Returns tha transform aToC
//
Transform multiplyTransforms(Transform aToB, Transform bToC);

// TODO: put this in camera component code?
Mat4 worldToView(Transform* cameraXfm);
