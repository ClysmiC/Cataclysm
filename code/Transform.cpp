#include "Transform.h"

Transform::Transform()
    : Transform(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

Transform::Transform(Vec3 position)
    : Transform(position, Quaternion(), Vec3(1, 1, 1))
{
}

Transform::Transform(Vec3 position, Quaternion orientation)
    : Transform(position, orientation, Vec3(1, 1, 1))
{
}

Transform::Transform(LiteTransform& transform)
    : Transform(transform.position(), transform.orientation(), transform.scale())
{
}

Transform::Transform(Vec3 position, Quaternion orientation, Vec3 scale)
    : _localPosition(position), _localOrientation(orientation), _localScale(scale)
{
}

Mat4
Transform::matrix()
{
    if (dirty)
    {
        recalculateWorld();
        
        this->toWorld.identityInPlace(); // reset to identity
        
        this->toWorld.scaleInPlace(this->scale());
        this->toWorld.rotateInPlace(this->orientation());
        this->toWorld.translateInPlace(this->position());
    }
    
    return this->toWorld;
}

void
Transform::setLocalPosition(Vec3 position)
{
    _localPosition = position;
    markSelfAndChildrenDirty();
}

void
Transform::setLocalOrientation(Quaternion orientation)
{
    _localOrientation = orientation;
    markSelfAndChildrenDirty();
}
void
Transform::setLocalScale(Vec3 scale)
{
    _localScale = scale;
    markSelfAndChildrenDirty();
}
void
Transform::setPosition(Vec3 position)
{
    Transform* p = this->getParent();
    if (p)
    {
        Mat4 worldToParent = inverse(p->matrix());
        Vec4 newLocal = worldToParent * Vec4(position, 1.0);
        this->setLocalPosition(newLocal.xyz());
    }
    else
    {
        this->setLocalPosition(position);
    }
}

void
Transform::setOrientation(Quaternion orientation)
{
    Transform* p = this->getParent();
    if (p)
    {
        Quaternion rotationNeeded = relativeRotation(p->orientation(), orientation);
        this->setLocalOrientation(rotationNeeded);
    }
    else
    {
        this->setLocalOrientation(orientation);
    }
}

void
Transform::setScale(Vec3 scale)
{
    Transform* p = this->getParent();
    if (p)
    {
        this->setLocalScale(hadamardDivide(scale, p->scale()));
    }
    else
    {
        this->setLocalScale(scale);
    }
}

void
Transform::markSelfAndChildrenDirty()
{
    dirty = true;
    
    auto children = this->getChildren();
    for (auto t : children)
    {
        t->markSelfAndChildrenDirty();
    }
}

void
Transform::recalculateWorld()
{
    Transform* p = this->getParent();

    if (p)
    {
        multiplyTransforms(
            this->_localPosition, this->_localOrientation, this->_localScale,
            p->position(), p->orientation(), p->scale(),
            &this->worldPosition, &this->worldOrientation, &this->worldScale
        );
    }
    else
    {
        this->worldPosition = this->_localPosition;
        this->worldOrientation = this->_localOrientation;
        this->worldScale = this->_localScale;
    }

    dirty = false;
}

LiteTransform::LiteTransform()
    : LiteTransform(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

LiteTransform::LiteTransform(Vec3 position)
    : LiteTransform(position, Quaternion(), Vec3(1, 1, 1))
{
}

LiteTransform::LiteTransform(Vec3 position, Quaternion orientation)
    : LiteTransform(position, orientation, Vec3(1, 1, 1))
{
}

LiteTransform::LiteTransform(Transform& transform)
    : LiteTransform(transform.position(), transform.orientation(), transform.scale())
{
}

LiteTransform::LiteTransform(Vec3 position, Quaternion orientation, Vec3 scale)
    : _position(position), _orientation(orientation), _scale(scale)
{
}

void multiplyTransforms(
    Vec3 aToBPos, Quaternion aToBOrientation, Vec3 aToBScale,
    Vec3 bToCPos, Quaternion bToCOrientation, Vec3 bToCScale,
    Vec3* outPosition, Quaternion* outOrientation, Vec3* outScale
)
{
    *outOrientation = bToCOrientation * aToBOrientation;
    *outScale = hadamard(bToCScale, aToBScale);
    *outPosition = bToCOrientation * hadamard(bToCScale, aToBPos) + bToCPos;
}

Mat4 worldToView(LiteTransform* cameraXfm)
{
    Vec3 up = cameraXfm->up();
    Vec3 right = cameraXfm->right();
    Vec3 back = cameraXfm->back(); // NOTE: This is direction pointing TOWARDS the camera (i.e., -forward)

    Mat4 result;
    
    // Rotate
    result[0][0] = right.x;
    result[1][0] = up.x;
    result[2][0] = back.x;

    result[0][1] = right.y;
    result[1][1] = up.y;
    result[2][1] = back.y;
        
    result[0][2] = right.z;
    result[1][2] = up.z;
    result[2][2] = back.z;

    // Homogeneous
    result[3][3] = 1;
    
    // Translate
    Mat4 translation = translationMatrix(-cameraXfm->position());
    result = result * translation;

    return result;
}


