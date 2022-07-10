#pragma once

#include "Types.h"

namespace Cyclone
{

// #todo_math
struct Vec2
{
    float X = 0.f;
    float Y = 0.f;

    bool operator == (const Vec2& Other) const noexcept = default;
};

struct Vec3
{
    float X = 0.f;
    float Y = 0.f;
    float Z = 0.f;

    bool operator == (const Vec3& Other) const noexcept = default;
};

struct Vec4
{
    float X = 0.f;
    float Y = 0.f;
    float Z = 0.f;
    float W = 0.f;

    bool operator == (const Vec4& Other) const noexcept = default;
};

struct Vec2i
{
    int32 X = 0;
    int32 Y = 0;

    bool operator == (const Vec2i& Other) const noexcept = default;
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
    
    bool operator == (const Mat44& Other) const noexcept = default;
};

template<>
// format is (x,y) without spaces
inline bool ConvertFromString(std::string_view Data, Vec2& Value)
{
    int Ret = sscanf_s(Data.data(), "(%f,%f)", &Value.X, &Value.Y);
    return Ret == 2;
}

template<>
// format is (x,y,z) without spaces
inline bool ConvertFromString(std::string_view Data, Vec3& Value)
{
    int Ret = sscanf_s(Data.data(), "(%f,%f,%f)", &Value.X, &Value.Y, &Value.Z);
    return Ret == 3;
}

template<>
// format is (x,y) without spaces
inline bool ConvertFromString(std::string_view Data, Vec2i& Value)
{
    int Ret = sscanf_s(Data.data(), "(%d,%d)", &Value.X, &Value.Y);
    return Ret == 2;
}
} // namespace Cyclone
