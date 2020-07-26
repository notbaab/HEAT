#pragma once

#include <math.h>
#include <stdio.h>

class Vector3
{
  public:
    float x, y, z;

    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

    void Set(float x, float y, float z)
    {
        x = x;
        y = y;
        z = z;
    }

    friend Vector3 operator+(const Vector3& inLeft, const Vector3& inRight)
    {
        return Vector3(inLeft.x + inRight.x, inLeft.y + inRight.y, inLeft.z + inRight.z);
    }

    friend Vector3 operator-(const Vector3& inLeft, const Vector3& inRight)
    {
        return Vector3(inLeft.x - inRight.x, inLeft.y - inRight.y, inLeft.z - inRight.z);
    }

    // Component-wise multiplication
    friend Vector3 operator*(const Vector3& inLeft, const Vector3& inRight)
    {
        return Vector3(inLeft.x * inRight.x, inLeft.y * inRight.y, inLeft.z * inRight.z);
    }

    // Scalar multiply
    friend Vector3 operator*(float inScalar, const Vector3& inVec)
    {
        return Vector3(inVec.x * inScalar, inVec.y * inScalar, inVec.z * inScalar);
    }

    friend Vector3 operator*(const Vector3& inVec, float inScalar)
    {
        return Vector3(inVec.x * inScalar, inVec.y * inScalar, inVec.z * inScalar);
    }

    Vector3& operator*=(float inScalar)
    {
        x *= inScalar;
        y *= inScalar;
        z *= inScalar;
        return *this;
    }

    Vector3& operator+=(const Vector3& inRight)
    {
        x += inRight.x;
        y += inRight.y;
        z += inRight.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& inRight)
    {
        x -= inRight.x;
        y -= inRight.y;
        z -= inRight.z;
        return *this;
    }

    float Length() { return sqrtf(x * x + y * y + z * z); }

    float LengthSq() { return x * x + y * y + z * z; }

    float Length2D() { return sqrtf(x * x + y * y); }

    float LengthSq2D() { return x * x + y * y; }

    void Normalize()
    {
        float length = Length();
        x /= length;
        y /= length;
        z /= length;
    }

    void Normalize2D()
    {
        float length = Length2D();
        x /= length;
        y /= length;
    }

    friend float Dot(const Vector3& inLeft, const Vector3& inRight)
    {
        return (inLeft.x * inRight.x + inLeft.y * inRight.y + inLeft.z * inRight.z);
    }

    friend float Dot2D(const Vector3& inLeft, const Vector3& inRight)
    {
        return (inLeft.x * inRight.x + inLeft.y * inRight.y);
    }

    friend Vector3 Cross(const Vector3& inLeft, const Vector3& inRight)
    {
        Vector3 temp;
        temp.x = inLeft.y * inRight.z - inLeft.z * inRight.y;
        temp.y = inLeft.z * inRight.x - inLeft.x * inRight.z;
        temp.z = inLeft.x * inRight.y - inLeft.y * inRight.x;
        return temp;
    }

    friend Vector3 Lerp(const Vector3& inA, const Vector3& inB, float t) { return Vector3(inA + t * (inB - inA)); }

    static const Vector3 Zero;
    static const Vector3 UnitX;
    static const Vector3 UnitY;
    static const Vector3 UnitZ;
};

enum class MovementType
{
    NONE,
    IDLE,
    WALK,
    ATTACK,
};

enum class MovementOrientation
{
    NONE,
    UP,
    UP_LEFT,
    UP_RIGHT,
    DOWN,
    DOWN_LEFT,
    DOWN_RIGHT,
    LEFT,
    RIGHT,
};

MovementOrientation MovementOrientationFromString(std::string orientationStr);
MovementType MovementTypeFromString(std::string animType);
MovementOrientation OrientationFromVector(Vector3 movement);

namespace Math
{
const float PI = 3.1415926535f;
float GetRandomFloat();

Vector3 GetRandomVector(const Vector3& inMin, const Vector3& inMax);

inline bool Is2DVectorEqual(const Vector3& inA, const Vector3& inB) { return (inA.x == inB.x && inA.y == inB.y); }

inline float ToDegrees(float inRadians) { return inRadians * 180.0f / PI; }
} // namespace Math
