#include "als_math.h"
#include <cmath>

Vec2::Vec2(float32 value) { this->x = value; this->y = value; }
Vec2::Vec2(float32 x, float32 y) { this->x = x; this->y = y; }

Vec3::Vec3(Axis3D unitAxis)
{
    switch (unitAxis) {
        case Axis3D::X: this->x = 1; this->y = 0; this->z = 0; break;
        case Axis3D::Y: this->x = 0; this->y = 1; this->z = 0; break;
        case Axis3D::Z: this->x = 0; this->y = 0; this->z = 1; break;
        default: assert(false);
    }
}

Vec3::Vec3(float32 value) { this->x = value; this->y = value; this->z = value; }
Vec3::Vec3(Vec2 xy, float32 z) { this->x = xy.x; this->y = xy.y; this->z = z; }
Vec3::Vec3(float32 x, float32 y, float32 z) { this->x = x; this->y = y; this->z = z; }

Vec4::Vec4(float32 value) { this->x = value; this->y = value; this->z = value; }
Vec4::Vec4(Vec2 xy, float32 z, float32 w) { this->x = xy.x; this->y = xy.y; this->z = z; this->w = w; }
Vec4::Vec4(Vec2 xy, Vec2 zw) { this->x = xy.x; this->y = xy.y; this->z = zw.x; this->w = zw.y; }
Vec4::Vec4(Vec3 xyz, float32 w) { this->x = xyz.x; this->y = xyz.y; this->z = xyz.z; this->w = w; }
Vec4::Vec4(float32 x, float32 y, float32 z, float32 w) { this->x = x; this->y = y; this->z = z; this->w = w; }

Quaternion::Quaternion()                           { x = 0; y = 0; z = 0; w = 1;}
Quaternion::Quaternion(Vec3 vector, float32 scalar) { x = vector.x; y = vector.y; z = vector.z; w = scalar; }
Quaternion::Quaternion(Vec4 xyzw)                  { x = xyzw.x; y = xyzw.y; z = xyzw.z; w = xyzw.w;}
Quaternion::Quaternion(float32 x_, float32 y_, float32 z_, float32 w_) { x = x_; y = y_; z = z_; w = w_; }

Mat3::Mat3(Mat4 mat4)
{
    _e[0][0] = mat4[0][0];
    _e[0][1] = mat4[0][1];
    _e[0][2] = mat4[0][2];

    _e[1][0] = mat4[1][0];
    _e[1][1] = mat4[1][1];
    _e[1][2] = mat4[1][2];

    _e[2][0] = mat4[2][0];
    _e[2][1] = mat4[2][1];
    _e[2][2] = mat4[2][2];
}

Mat4::Mat4(Mat3 mat3)
{
    _e[0][0] = mat3[0][0];
    _e[0][1] = mat3[0][1];
    _e[0][2] = mat3[0][2];
    _e[0][3] = 0;

    _e[1][0] = mat3[1][0];
    _e[1][1] = mat3[1][1];
    _e[1][2] = mat3[1][2];
    _e[1][3] = 0;

    _e[2][0] = mat3[2][0];
    _e[2][1] = mat3[2][1];
    _e[2][2] = mat3[2][2];
    _e[2][3] = 0;

    _e[3][0] = 0;
    _e[3][1] = 0;
    _e[3][2] = 0;
    _e[3][3] = 1;
}

Plane::Plane(Vec3 point, Vec3 normal)
    : point(point)
    , normal(normalize(normal))
{
}


Plane::Plane(Vec3 pointA, Vec3 pointB, Vec3 pointC)
{
    Vec3 ab = pointB - pointA;
    Vec3 ac = pointC - pointA;
    
    point = pointA;
    normal = normalize(cross(ab, ac));
}

Vec2& Vec2::normalizeInPlace()
{
    float32 len = length(*this);
    x /= len;
    y /= len;

    return *this;
}

Vec2& Vec2::normalizeOrXAxisInPlace()
{
    float32 len = length(*this);

    if (len < EPSILON)
    {
        x = 1;
        y = 0;
    }
    else
    {
        x /= len;
        y /= len;
    }

    return *this;
}

Vec3& Vec3::normalizeInPlace()
{
    float32 len = length(*this);
    x /= len;
    y /= len;
    z /= len;

    return *this;
}

Vec3& Vec3::normalizeOrXAxisInPlace()
{
    float32 len = length(*this);

    if (len < EPSILON)
    {
        x = 1;
        y = 0;
        z = 0;
    }
    else
    {
        x /= len;
        y /= len;
        z /= len;
    }

    return *this;
}

Vec4& Vec4::normalizeInPlace()
{
    float32 len = length(*this);
    x /= len;
    y /= len;
    z /= len;
    w /= len;

    return *this;
}

Quaternion& Quaternion::normalizeInPlace()
{
    float32 len = length(*this);
    x /= len;
    y /= len;
    z /= len;
    w /= len;

    return *this;
}



float32* Mat3::operator [] (int32 i)
{
    assert(i < 3);
    float32* result = _e[i];
    return result;
}

float32* Mat3::dataPointer()
{
    float32* result = _e[0];
    return result;
}

Mat3& Mat3::identityInPlace()
{
    (*this)[0][0] = 1;
    (*this)[0][1] = 0;
    (*this)[0][2] = 0;

    (*this)[1][0] = 0;
    (*this)[1][1] = 1;
    (*this)[1][2] = 0;

    (*this)[2][0] = 0;
    (*this)[2][1] = 0;
    (*this)[2][2] = 1;

    return *this;
}

Mat3& Mat3::transposeInPlace()
{
    float32 temp;

    temp = (*this)[0][1];
    (*this)[0][1] = (*this)[1][0];
    (*this)[1][0] = temp;

    temp = (*this)[0][2];
    (*this)[0][2] = (*this)[2][0];
    (*this)[2][0] = temp;

    temp = (*this)[1][2];
    (*this)[1][2] = (*this)[2][1];
    (*this)[2][1] = temp;

    return *this;
}

float32* Mat4::operator [] (int32 i)
{
    assert(i < 4);
    float32* result = _e[i];
    return result;
}

float32* Mat4::dataPointer()
{
    float32* result = _e[0];
    return result;
}

Mat4& Mat4::scaleInPlace(Vec3 scale)
{
    Mat4 sm = scalingMatrix(scale);
    *this = sm * *this;
    return *this;
}

Mat4& Mat4::rotateInPlace(Quaternion rotation)
{
    Mat4 rm = rotationMatrix4(rotation);
    *this = rm * *this;
    return *this;
}

Mat4& Mat4::translateInPlace(Vec3 translation)
{
    Mat4 tm = translationMatrix(translation);
    *this = tm * *this;
    return *this;
}

Mat4& Mat4::identityInPlace()
{
    (*this)[0][0] = 1;
    (*this)[0][1] = 0;
    (*this)[0][2] = 0;
    (*this)[0][3] = 0;

    (*this)[1][0] = 0;
    (*this)[1][1] = 1;
    (*this)[1][2] = 0;
    (*this)[1][3] = 0;

    (*this)[2][0] = 0;
    (*this)[2][1] = 0;
    (*this)[2][2] = 1;
    (*this)[2][3] = 0;

    (*this)[3][0] = 0;
    (*this)[3][1] = 0;
    (*this)[3][2] = 0;
    (*this)[3][3] = 1;

    return *this;
}

Mat4& Mat4::transposeInPlace()
{
    float32 temp;

    temp = (*this)[0][1];
    (*this)[0][1] = (*this)[1][0];
    (*this)[1][0] = temp;

    temp = (*this)[0][2];
    (*this)[0][2] = (*this)[2][0];
    (*this)[2][0] = temp;

    temp = (*this)[1][2];
    (*this)[1][2] = (*this)[2][1];
    (*this)[2][1] = temp;

    temp = (*this)[0][3];
    (*this)[0][3] = (*this)[3][0];
    (*this)[3][0] = temp;

    temp = (*this)[1][3];
    (*this)[1][3] = (*this)[3][1];
    (*this)[3][1] = temp;

    temp = (*this)[2][3];
    (*this)[2][3] = (*this)[3][2];
    (*this)[3][2] = temp;

    return *this;
}

Mat4& Mat4::mat3ifyInPlace()
{
    (*this)[0][3] = 0;
    (*this)[1][3] = 0;
    (*this)[2][3] = 0;

    (*this)[3][0] = 0;
    (*this)[3][1] = 0;
    (*this)[3][2] = 0;

    (*this)[3][3] = 1;

    return *this;
}

Mat4& Mat4::perspectiveInPlace(float32 fov, float32 aspectRatio, float32 near, float32 far)
{
    assert(fov > 0);
    assert(aspectRatio > 0);
    assert(near > 0);
    assert(far > 0);
    assert(far > near);

    float32 halfFov = fov / 2;
    float32 right = near * tanf(TO_RAD(halfFov));
    float32 top = right / (aspectRatio);

    (*this)[0][0] = near / right;
    (*this)[0][1] = 0;
    (*this)[0][2] = 0;
    (*this)[0][3] = 0;

    (*this)[1][0] = 0;
    (*this)[1][1] = near / top;
    (*this)[1][2] = 0;
    (*this)[1][3] = 0;

    (*this)[2][0] = 0;
    (*this)[2][1] = 0;
    (*this)[2][2] = -(far + near) / (far - near);
    (*this)[2][3] = -2 * far * near / (far - near);

    (*this)[3][0] = 0;
    (*this)[3][1] = 0;
    (*this)[3][2] = -1;
    (*this)[3][3] = 0;
    
    return *this;
}

Mat4& Mat4::orthoInPlace(float32 width, float32 aspectRatio, float32 near, float32 far)
{
    assert(width > 0);
    assert(aspectRatio > 0);
    assert(near > 0);
    assert(far > 0);
    assert(far > near);

    float32 right = width / 2.0f;
    float32 top = right / (aspectRatio);

    (*this)[0][0] = 1 / right;
    (*this)[0][1] = 0;
    (*this)[0][2] = 0;
    (*this)[0][3] = 0;

    (*this)[1][0] = 0;
    (*this)[1][1] = 1 / top;
    (*this)[1][2] = 0;
    (*this)[1][3] = 0;

    (*this)[2][0] = 0;
    (*this)[2][1] = 0;
    (*this)[2][2] = -2 / (far - near);
    (*this)[2][3] = - (far + near) / (far - near);

    (*this)[3][0] = 0;
    (*this)[3][1] = 0;
    (*this)[3][2] = 0;
    (*this)[3][3] = 1;
    
    return *this;
}


//
// Operator overloads
//

// Vec2
Vec2 operator + (Vec2 vectorA, Vec2 vectorB)
{
    Vec2 result;

    result.x = vectorA.x + vectorB.x;
    result.y = vectorA.y + vectorB.y;
    
    return result;
}
Vec2 operator + (Vec2 vector, float32 constant)
{
    Vec2 result;

    result.x = vector.x + constant;
    result.y = vector.y + constant;
    
    return result;
}
Vec2& operator += (Vec2& vectorA, Vec2 vectorB)
{
    vectorA.x += vectorB.x;
    vectorA.y += vectorB.y;

    return vectorA;
}
Vec2& operator += (Vec2& vector, float32 constant)
{
    vector = vector + constant;
    return vector;
}

Vec2 operator - (Vec2 vectorA, Vec2 vectorB)
{
    Vec2 result;

    result.x = vectorA.x - vectorB.x;
    result.y = vectorA.y - vectorB.y;
    
    return result;
}
Vec2 operator - (Vec2 vector, float32 constant)
{
    Vec2 result;

    result.x = vector.x - constant;
    result.y = vector.y - constant;
    
    return result;
}
Vec2& operator -= (Vec2& vectorA, Vec2 vectorB)
{
    vectorA.x -= vectorB.x;
    vectorA.y -= vectorB.y;

    return vectorA;
}
Vec2& operator -= (Vec2& vector, float32 constant)
{
    vector = vector - constant;
    return vector;
}
Vec2 operator - (Vec2 vector)
{
    Vec2 result;
    result.x = -vector.x;
    result.y = -vector.y;

    return result;
}


Vec2 operator * (float32 scalar, Vec2 vector)
{
    Vec2 result;

    result.x = scalar * vector.x;
    result.y = scalar * vector.y;

    return result;
}
Vec2 operator * (Vec2 vector, float32 scalar)
{
    return scalar * vector;
}
Vec2& operator *= (Vec2& vector, float32 scalar)
{
    vector = vector * scalar;
    return vector;
}

Vec2 operator / (Vec2 vector, float32 scalar)
{
    Vec2 result;

    result.x = vector.x / scalar;
    result.y = vector.y / scalar;

    return result;
}
Vec2& operator /= (Vec2& vector, float32 scalar)
{
    vector = vector / scalar;
    return vector;
}


// Vec3
Vec3 operator + (Vec3 vectorA, Vec3 vectorB)
{
    Vec3 result;

    result.x = vectorA.x + vectorB.x;
    result.y = vectorA.y + vectorB.y;
    result.z = vectorA.z + vectorB.z;
    
    return result;
}
Vec3 operator + (Vec3 vector, float32 constant)
{
    Vec3 result;

    result.x = vector.x + constant;
    result.y = vector.y + constant;
    result.z = vector.z + constant;
    
    return result;
}
Vec3& operator += (Vec3& vectorA, Vec3 vectorB)
{
    vectorA.x += vectorB.x;
    vectorA.y += vectorB.y;
    vectorA.z += vectorB.z;    

    return vectorA;
}
Vec3& operator += (Vec3& vector, float32 constant)
{
    vector = vector + constant;
    return vector;
}

Vec3 operator - (Vec3 vectorA, Vec3 vectorB)
{
    Vec3 result;

    result.x = vectorA.x - vectorB.x;
    result.y = vectorA.y - vectorB.y;
    result.z = vectorA.z - vectorB.z;
    
    return result;
}
Vec3 operator - (Vec3 vector, float32 constant)
{
    Vec3 result;

    result.x = vector.x - constant;
    result.y = vector.y - constant;
    result.z = vector.z - constant;
    
    return result;
}
Vec3& operator -= (Vec3& vectorA, Vec3 vectorB)
{
    vectorA.x -= vectorB.x;
    vectorA.y -= vectorB.y;
    vectorA.z -= vectorB.z;

    return vectorA;
}
Vec3& operator -= (Vec3& vector, float32 constant)
{
    vector = vector - constant;
    return vector;
}
Vec3 operator - (Vec3 vector)
{
    Vec3 result;
    result.x = -vector.x;
    result.y = -vector.y;
    result.z = -vector.z;

    return result;
}


Vec3 operator * (float32 scalar, Vec3 vector)
{
    Vec3 result;

    result.x = scalar * vector.x;
    result.y = scalar * vector.y;
    result.z = scalar * vector.z;

    return result;
}
Vec3 operator * (Vec3 vector, float32 scalar)
{
    return scalar * vector;
}
Vec3& operator *= (Vec3& vector, float32 scalar)
{
    vector = vector * scalar;
    return vector;
}

Vec3 operator / (Vec3 vector, float32 scalar)
{
    Vec3 result;

    result.x = vector.x / scalar;
    result.y = vector.y / scalar;
    result.z = vector.z / scalar;

    return result;
}
Vec3& operator /= (Vec3& vector, float32 scalar)
{
    vector = vector / scalar;
    return vector;
}

// Vec4
Vec4 operator + (Vec4 vectorA, Vec4 vectorB)
{
    Vec4 result;

    result.x = vectorA.x + vectorB.x;
    result.y = vectorA.y + vectorB.y;
    result.z = vectorA.z + vectorB.z;
    result.w = vectorA.w + vectorB.w;
    
    return result;
}
Vec4 operator + (Vec4 vector, float32 constant)
{
    Vec4 result;

    result.x = vector.x + constant;
    result.y = vector.y + constant;
    result.z = vector.z + constant;
    result.w = vector.w + constant;
    
    return result;
}
Vec4& operator += (Vec4& vectorA, Vec4 vectorB)
{
    vectorA.x += vectorB.x;
    vectorA.y += vectorB.y;
    vectorA.z += vectorB.z;
    vectorA.w += vectorB.w;


    return vectorA;
}
Vec4& operator += (Vec4& vector, float32 constant)
{
    vector = vector + constant;
    return vector;
}

Vec4 operator - (Vec4 vectorA, Vec4 vectorB)
{
    Vec4 result;

    result.x = vectorA.x - vectorB.x;
    result.y = vectorA.y - vectorB.y;
    result.z = vectorA.z - vectorB.z;
    result.w = vectorA.w - vectorB.w;
    
    return result;
}
Vec4 operator - (Vec4 vector, float32 constant)
{
    Vec4 result;

    result.x = vector.x - constant;
    result.y = vector.y - constant;
    result.z = vector.z - constant;
    result.w = vector.w - constant;
    
    return result;
}
Vec4& operator -= (Vec4& vectorA, Vec4 vectorB)
{
    vectorA.x -= vectorB.x;
    vectorA.y -= vectorB.y;
    vectorA.z -= vectorB.z;
    vectorA.w -= vectorB.w;

    return vectorA;
}
Vec4& operator -= (Vec4& vector, float32 constant)
{
    vector = vector - constant;
    return vector;
}
Vec4 operator - (Vec4 vector)
{
    Vec4 result;
    result.x = -vector.x;
    result.y = -vector.y;
    result.z = -vector.z;
    result.w = -vector.w;

    return result;
}


Vec4 operator * (float32 scalar, Vec4 vector)
{
    Vec4 result;

    result.x = scalar * vector.x;
    result.y = scalar * vector.y;
    result.z = scalar * vector.z;
    result.w = scalar * vector.w;

    return result;
}
Vec4 operator * (Vec4 vector, float32 scalar)
{
    return scalar * vector;
}
Vec4& operator *= (Vec4& vector, float32 scalar)
{
    vector = vector * scalar;
    return vector;
}

Vec4 operator / (Vec4 vector, float32 scalar)
{
    Vec4 result;

    result.x = vector.x / scalar;
    result.y = vector.y / scalar;
    result.z = vector.z / scalar;
    result.w = vector.w / scalar;

    return result;
}
Vec4& operator /= (Vec4& vector, float32 scalar)
{
    vector = vector / scalar;
    return vector;
}

// Quaternion
Quaternion operator + (Quaternion quatA, Quaternion quatB)
{
    Quaternion result;

    result.x = quatA.x + quatB.x;
    result.y = quatA.y + quatB.y;
    result.z = quatA.z + quatB.z;
    result.w = quatA.w + quatB.w;
    
    return result;
}
Quaternion operator + (Quaternion quat, float32 constant)
{
    Quaternion result;

    result.x = quat.x + constant;
    result.y = quat.y + constant;
    result.z = quat.z + constant;
    result.w = quat.w + constant;
    
    return result;
}
Quaternion& operator += (Quaternion& quatA, Quaternion quatB)
{
    quatA.x += quatB.x;
    quatA.y += quatB.y;
    quatA.z += quatB.z;
    quatA.w += quatB.w;


    return quatA;
}
Quaternion& operator += (Quaternion& quat, float32 constant)
{
    quat = quat + constant;
    return quat;
}

Quaternion operator - (Quaternion quatA, Quaternion quatB)
{
    Quaternion result;

    result.x = quatA.x - quatB.x;
    result.y = quatA.y - quatB.y;
    result.z = quatA.z - quatB.z;
    result.w = quatA.w - quatB.w;
    
    return result;
}
Quaternion operator - (Quaternion quat, float32 constant)
{
    Quaternion result;

    result.x = quat.x - constant;
    result.y = quat.y - constant;
    result.z = quat.z - constant;
    result.w = quat.w - constant;
    
    return result;
}
Quaternion& operator -= (Quaternion& quatA, Quaternion quatB)
{
    quatA.x -= quatB.x;
    quatA.y -= quatB.y;
    quatA.z -= quatB.z;
    quatA.w -= quatB.w;

    return quatA;
}
Quaternion& operator -= (Quaternion& quat, float32 constant)
{
    quat = quat - constant;
    return quat;
}
Quaternion operator - (Quaternion quat)
{
    Quaternion result;
    result.x = -quat.x;
    result.y = -quat.y;
    result.z = -quat.z;
    result.w = -quat.w;

    return result;
}


Quaternion operator * (float32 scalar, Quaternion quat)
{
    Quaternion result;

    result.x = scalar * quat.x;
    result.y = scalar * quat.y;
    result.z = scalar * quat.z;
    result.w = scalar * quat.w;

    return result;
}
Quaternion operator * (Quaternion quat, float32 scalar)
{
    return scalar * quat;
}
Quaternion& operator *= (Quaternion& quat, float32 scalar)
{
    quat = quat * scalar;
    return quat;
}

Quaternion operator / (Quaternion quat, float32 scalar)
{
    Quaternion result;

    result.x = quat.x / scalar;
    result.y = quat.y / scalar;
    result.z = quat.z / scalar;
    result.w = quat.w / scalar;

    return result;
}
Quaternion& operator /= (Quaternion& quat, float32 scalar)
{
    quat = quat / scalar;
    return quat;
}

Mat3 operator * (Mat3 left, Mat3 right)
{
    Mat3 result;

    const int matDimension = 3;

    for(int down = 0; down < matDimension; down++)
    {
        for(int across = 0; across < matDimension; across++)
        {
            float32 sum = 0;
            
            for(int i = 0; i < matDimension; i++)
            {
                sum += (left[down][i] * right[i][across]);
            }
            
            result[down][across] = sum;
        }
    }
    
    return result;
}

Mat3 operator * (Mat3 left, float32 right)
{
    Mat3 result;
    
    for (uint32 down = 0; down < 3; down++)
    {
        for (uint32 across = 0; across < 3; across++)
        {
            result[down][across] = left[down][across] * right;
        }
    }

    return result;
}

Mat3 operator * (float32 left, Mat3 right)
{
    return right * left;
}

Mat4 operator * (Mat4 left, Mat4 right)
{
    Mat4 result;

    const int matDimension = 4;

    for(int down = 0; down < matDimension; down++)
    {
        for(int across = 0; across < matDimension; across++)
        {
            float32 sum = 0;
            
            for(int i = 0; i < matDimension; i++)
            {
                sum += (left[down][i] * right[i][across]);
            }
            
            result[down][across] = sum;
        }
    }
    
    return result;
}

Mat4 operator * (Mat4 left, float32 right)
{
    Mat4 result;
    
    for (uint32 down = 0; down < 4; down++)
    {
        for (uint32 across = 0; across < 4; across++)
        {
            result[down][across] = left[down][across] * right;
        }
    }

    return result;
}

Mat4 operator * (float32 left, Mat4 right)
{
    return right * left;
}

Vec3 operator * (Mat3 m, Vec3 v)
{
    Vec3 result;

    result.x = m[0][0] * v.x   +   m[0][1] * v.y   +   m[0][2] * v.z;
    result.y = m[1][0] * v.x   +   m[1][1] * v.y   +   m[1][2] * v.z;
    result.z = m[2][0] * v.x   +   m[2][1] * v.y   +   m[2][2] * v.z;

    return result;
}

Vec4 operator * (Mat4 m, Vec4 v)
{
    Vec4 result;

    result.x = m[0][0] * v.x   +   m[0][1] * v.y   +   m[0][2] * v.z   +   m[0][3] * v.w;
    result.y = m[1][0] * v.x   +   m[1][1] * v.y   +   m[1][2] * v.z   +   m[1][3] * v.w;
    result.z = m[2][0] * v.x   +   m[2][1] * v.y   +   m[2][2] * v.z   +   m[2][3] * v.w;
    result.w = m[3][0] * v.x   +   m[3][1] * v.y   +   m[3][2] * v.z   +   m[3][3] * v.w;

    return result;
}

Quaternion operator * (Quaternion a, Quaternion b)
{
    Quaternion result;

    result.x = (a.w * b.x)   +   (a.x * b.w)   +   (a.y * b.z)   -   (a.z * b.y);
    result.y = (a.w * b.y)   +   (a.y * b.w)   +   (a.z * b.x)   -   (a.x * b.z);
    result.z = (a.w * b.z)   +   (a.z * b.w)   +   (a.x * b.y)   -   (a.y * b.x);
    result.w = (a.w * b.w)   -   (a.x * b.x)   -   (a.y * b.y)   -   (a.z * b.z);

    return result;
}

Vec3 operator * (Quaternion quaternion, Vec3 vector)
{
    Vec3 result = rotationMatrix3(quaternion) * vector;

    return result;
}


// Taken from http://answers.unity3d.com/questions/467614/what-is-the-source-code-of-quaternionlookrotation.html
// Don't really understand how it works......
Quaternion lookRotation(Vec3 forward, Vec3 up)
{
    assert(isOrthogonal(forward, up));
               
    forward = normalize(forward);
 
    Vec3 vector = normalize(forward);
    Vec3 vector2 = normalize(cross(up, vector));
    Vec3 vector3 = cross(vector, vector2);
    float32 m00 = vector2.x;
    float32 m01 = vector2.y;
    float32 m02 = vector2.z;
    float32 m10 = vector3.x;
    float32 m11 = vector3.y;
    float32 m12 = vector3.z;
    float32 m20 = vector.x;
    float32 m21 = vector.y;
    float32 m22 = vector.z;
 
 
    float32 num8 = (m00 + m11) + m22;
    
    Quaternion quaternion;
    
    if (num8 > 0.0f)
    {
        float32 num = (float32)sqrt(num8 + 1.0f);
        quaternion.w = num * 0.5f;
        num = 0.5f / num;
        quaternion.x = (m12 - m21) * num;
        quaternion.y = (m20 - m02) * num;
        quaternion.z = (m01 - m10) * num;
        return quaternion;
    }
    if ((m00 >= m11) && (m00 >= m22))
    {
        float32 num7 = (float32)sqrt(((1.0f + m00) - m11) - m22);
        float32 num4 = 0.5f / num7;
        quaternion.x = 0.5f * num7;
        quaternion.y = (m01 + m10) * num4;
        quaternion.z = (m02 + m20) * num4;
        quaternion.w = (m12 - m21) * num4;
        return quaternion;
    }
    if (m11 > m22)
    {
        float32 num6 = (float32)sqrt(((1.0f + m11) - m00) - m22);
        float32 num3 = 0.5f / num6;
        quaternion.x = (m10+ m01) * num3;
        quaternion.y = 0.5f * num6;
        quaternion.z = (m21 + m12) * num3;
        quaternion.w = (m20 - m02) * num3;
        return quaternion; 
    }
    float32 num5 = (float32)sqrt(((1.0f + m22) - m00) - m11);
    float32 num2 = 0.5f / num5;
    quaternion.x = (m20 + m02) * num2;
    quaternion.y = (m21 + m12) * num2;
    quaternion.z = 0.5f * num5;
    quaternion.w = (m01 - m10) * num2;
    
    return normalize(quaternion);
}

Quaternion relativeRotation(Vec3 start, Vec3 end)
{
    Quaternion result;
    start.normalizeInPlace();
    end.normalizeInPlace();
    
    float32 startDotEnd = dot(start, end);
    
    if (startDotEnd >= 1.0f)
    {
        return result;
    }
    
    if (startDotEnd <= -1.0f)
    {
        Vec3 someVector = someNonParallelAxis(start);
        Vec3 axis = cross(start, someVector);
        return axisAngle(axis, 180);
    }

    Vec3 axis = cross(start, end);
    result.x = axis.x;
    result.y = axis.y;
    result.z = axis.z;
    result.w = std::sqrt(lengthSquared(start) * lengthSquared(end)) + startDotEnd;

    result.normalizeInPlace();

    return result;
}

Vec3 toEuler(Mat4 m)
{
    // See https://github.com/mrdoob/three.js/blob/dev/src/math/Euler.js
    // Note: assumes pure (unscaled) rotation matrix
    
    Vec3 result;
    
    result.y = TO_DEG(asin(clamp(m[0][2], -1, 1)));

    if (fabs(m[0][2]) < 0.99999)
    {
        result.x = TO_DEG(atan2(-m[1][2], m[2][2]));
        result.z = TO_DEG(atan2(-m[0][1], m[0][0]));
    }
    else
    {
        result.x = TO_DEG(atan2(m[2][1], m[1][1]));
        result.z = 0;
    }

    return result;
}

Vec3 toEuler(Quaternion q)
{
    // See https://github.com/mrdoob/three.js/blob/dev/src/math/Euler.js

    Mat4 m;
    m.rotateInPlace(q);

    Vec3 result = toEuler(m);

    return result;
}

Quaternion fromEuler(Vec3 euler)
{
    // See https://github.com/mrdoob/three.js/blob/dev/src/math/Quaternion.js

    float32 xRad = TO_RAD(euler.x);
    float32 yRad = TO_RAD(euler.y);
    float32 zRad = TO_RAD(euler.z);
    
    float32 halfCosX   = cos(xRad * 0.5);
    float32 halfCosY   = cos(yRad * 0.5);
    float32 halfCosZ   = cos(zRad * 0.5);
    float32 halfSinX   = sin(xRad * 0.5);
    float32 halfSinY   = sin(yRad * 0.5);
    float32 halfSinZ   = sin(zRad * 0.5);
    
    Quaternion q;
    q.w = (halfCosX * halfCosY * halfCosZ) - (halfSinX * halfSinY * halfSinZ);
    q.x = (halfSinX * halfCosY * halfCosZ) + (halfCosX * halfSinY * halfSinZ);
    q.y = (halfCosX * halfSinY * halfCosZ) - (halfSinX * halfCosY * halfSinZ);
    q.z = (halfCosX * halfCosY * halfSinZ) + (halfSinX * halfSinY * halfCosZ);
    
    return q;
}

Mat3 inverse(Mat3 m)
{
    // Optimized formula from FGED Vol 1, 2nd edition, formula (1.95) and Listing 1.10
    Vec3 a = Vec3(m[0][0], m[1][0], m[2][0]);
    Vec3 b = Vec3(m[0][1], m[1][1], m[2][1]);
    Vec3 c = Vec3(m[0][2], m[1][2], m[2][2]);

    Vec3 aCrossB = cross(a, b);
    Vec3 bCrossC = cross(b, c);
    Vec3 cCrossA = cross(c, a);

    // Note: determinant of 3x3 matrix equals its triple product ((a x b) . c)
    float32 det = dot(aCrossB, c);
    assert(det != 0);
    
    float32 invDet = 1.0 / det;

    Mat3 result;
    result[0][0] = bCrossC.x * invDet;
    result[0][1] = bCrossC.y * invDet;
    result[0][2] = bCrossC.z * invDet;

    result[1][0] = cCrossA.x * invDet;
    result[1][1] = cCrossA.y * invDet;
    result[1][2] = cCrossA.z * invDet;

    result[2][0] = aCrossB.x * invDet;
    result[2][1] = aCrossB.y * invDet;
    result[2][2] = aCrossB.z * invDet;

    return result;
}

Mat4 inverse(Mat4 m)
{
    // Optimized formula from FGED Vol 1, 2nd edition, formula (1.99) and Listing 1.11
    Vec3 a = Vec3(m[0][0], m[1][0], m[2][0]);
    Vec3 b = Vec3(m[0][1], m[1][1], m[2][1]);
    Vec3 c = Vec3(m[0][2], m[1][2], m[2][2]);
    Vec3 d = Vec3(m[0][3], m[1][3], m[2][3]);
    float32 x = m[3][0];
    float32 y = m[3][1];
    float32 z = m[3][2];
    float32 w = m[3][3];

    Vec3 s = cross(a, b);
    Vec3 t = cross(c, d);
    Vec3 u = y * a - x * b;
    Vec3 v = w * c - z * d;

    float32 det = (dot(s, v) + dot(t, u));
    assert(det != 0);
        
    float32 invDet = 1.0 / det;

    // Note: formula (1.99) doesn't do this multiplication, but this is the code's way of
    // distributing the invDet across each entry in the matrix (since each entry gets multiplied)
    // by either s, t, u, or v at some point
    s *= invDet;
    t *= invDet;
    u *= invDet;
    v *= invDet;

    Vec3 r0 = cross(b, v) + t * y;
    Vec3 r1 = cross(v, a) - t * x;
    Vec3 r2 = cross(d, u) + s * w;
    Vec3 r3 = cross(u, c) - s * z;

    Mat4 result;
    result[0][0] = r0.x;
    result[0][1] = r0.y;
    result[0][2] = r0.z;
    result[0][3] = -dot(b, t);

    result[1][0] = r1.x;
    result[1][1] = r1.y;
    result[1][2] = r1.z;
    result[1][3] = dot(a, t);

    result[2][0] = r2.x;
    result[2][1] = r2.y;
    result[2][2] = r2.z;
    result[2][3] = -dot(d, s);

    result[3][0] = r3.x;
    result[3][1] = r3.y;
    result[3][2] = r3.z;
    result[3][3] = dot(c, s);

    return result;
}

float32 determinant(Mat3 m)
{
    float32 result =
        m[0][0] * m[1][1] * m[2][2] +
        m[0][1] * m[1][2] * m[2][0] +
        m[0][2] * m[1][0] * m[2][1]
        -
        m[0][0] * m[1][2] * m[2][1] -
        m[0][1] * m[1][0] * m[2][2] -
        m[0][2] * m[1][1] * m[2][0];

    return result;
}

float32 determinant(Mat4 m)
{
    float32 result;

    // Transformation matrices have [0 0 0 1] as row 4, which lets us
    // optimize this case
    if (m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1)
    {
        // @Optimize: can copy/paste content of the determinant(Mat3) function to avoid constructing a new Mat3
        result = determinant(Mat3(m));
    }
    else
    {
        // Optimized formula from FGED Vol 1, 2nd edition, formula (1.98)
        Vec3 a = Vec3(m[0][0], m[1][0], m[2][0]);
        Vec3 b = Vec3(m[0][1], m[1][1], m[2][1]);
        Vec3 c = Vec3(m[0][2], m[1][2], m[2][2]);
        Vec3 d = Vec3(m[0][3], m[1][3], m[2][3]);
        float32 x = m[3][0];
        float32 y = m[3][1];
        float32 z = m[3][2];
        float32 w = m[3][3];

        Vec3 s = cross(a, b);
        Vec3 t = cross(c, d);
        Vec3 u = y * a - x * b;
        Vec3 v = w * c - z * d;

        result = dot(s, v) + dot(t, u);


        // Slow version
        // result =
        //     m[0][0] * m[1][1] * m[2][2] * m[3][3] +
        //     m[0][0] * m[1][2] * m[2][3] * m[3][1] +
        //     m[0][0] * m[1][3] * m[2][1] * m[3][2] +
        //     m[0][1] * m[1][0] * m[2][3] * m[3][2] +
        //     m[0][1] * m[1][2] * m[2][0] * m[3][3] +
        //     m[0][1] * m[1][3] * m[2][2] * m[3][0] +
        //     m[0][2] * m[1][0] * m[2][1] * m[3][3] +
        //     m[0][2] * m[1][1] * m[2][3] * m[3][0] +
        //     m[0][2] * m[1][3] * m[2][0] * m[3][1] +
        //     m[0][3] * m[1][0] * m[2][2] * m[3][1] +
        //     m[0][3] * m[1][1] * m[2][0] * m[3][2] +
        //     m[0][3] * m[1][2] * m[2][1] * m[3][0]
        //     -
        //     m[0][0] * m[1][1] * m[2][3] * m[3][2] -
        //     m[0][0] * m[1][2] * m[2][1] * m[3][3] -
        //     m[0][0] * m[1][3] * m[2][2] * m[3][1] -
        //     m[0][1] * m[1][0] * m[2][2] * m[3][3] -
        //     m[0][1] * m[1][2] * m[2][3] * m[3][0] -
        //     m[0][1] * m[1][3] * m[2][0] * m[3][2] -
        //     m[0][2] * m[1][0] * m[2][3] * m[3][1] -
        //     m[0][2] * m[1][1] * m[2][0] * m[3][3] -
        //     m[0][2] * m[1][3] * m[2][1] * m[3][0] -
        //     m[0][3] * m[1][0] * m[2][1] * m[3][2] -
        //     m[0][3] * m[1][1] * m[2][2] * m[3][0] -
        //     m[0][3] * m[1][2] * m[2][0] * m[3][1];
    }

    return result;
}
