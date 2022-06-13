#pragma once

#include "Types.h"

namespace Cyclone
{

// #todo_math
struct Vec2
{
    float X;
    float Y;

    bool operator == (const Vec2& other) const noexcept
    {
        return X == other.X && Y == other.Y;
    }
};
struct Vec3
{
    float X;
    float Y;
    float Z;

    bool operator == (const Vec3& other) const noexcept
    {
        return X == other.X && Y == other.Y && Z == other.Z;
    }
};

struct Vec4
{
    float X;
    float Y;
    float Z;
    float W;

    bool operator == (const Vec4& other) const noexcept
    {
        return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
    }
};

// #todo_math assume row-major for now
struct Mat44
{
    Vec4 X;
    Vec4 Y;
    Vec4 Z;
    Vec4 W;

    Mat44 Invert() const { return *this; } // #todo_math
    static Mat44 CreateLookAtRH(const Vec3& ViewerPos, const Vec3& LookAtPos, const Vec3& Up) { return Mat44{}; }
    static Mat44 CreatePerspectiveFieldOfView(float vFov, float AspectRatio, float NearPlane, float FarPlane) { return Mat44{}; }
    static Mat44 CreateOrthographic(float Width, float Height, float NearPlane, float FarPlane) { return Mat44{}; }
    static Mat44 CreateOrthographicOffCenter(float Left, float Right, float Bottom, float Top, float NearPlane, float FarPlane) { return Mat44{}; }
};

} // namespace Cyclone
