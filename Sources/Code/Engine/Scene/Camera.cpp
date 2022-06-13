#include "Camera.h"

#include "Engine/Core/Helpers.h"

namespace Cyclone
{

CCamera::CCamera() = default;
CCamera::~CCamera() = default;

void CCamera::SetWorldTransform(const Mat44& worldTransform)
{
    m_Data.WorldMat = worldTransform;
    m_Data.ViewMat = m_Data.WorldMat.Invert();

    SetUpdated();
}

void CCamera::LookAtMatrix(const Vec3& viewerPos, const Vec3& lookAtPos)
{
    static const Vec3 Up{ 0.f, 1.f, 0.f };

    m_Data.ViewMat = Mat44::CreateLookAtRH(viewerPos, lookAtPos, Up);
    m_Data.WorldMat = m_Data.ViewMat.Invert();

    SetUpdated();
}

void CCamera::SetupPerspectiveProjectionMatrixRH(float vFov, float AspectRatio, float NearPlane, float FarPlane)
{
    m_Data.NearPlane = NearPlane;
    m_Data.FarPlane = FarPlane;
    m_Data.vFov = vFov;
    m_Data.AspectRatio = AspectRatio;

    // note that we use inverted z so 1 is near and 0 is far, and farPlane and nearPlane intended inverted in this call
    m_Data.ProjectionMat = Mat44::CreatePerspectiveFieldOfView(vFov, AspectRatio, FarPlane, NearPlane);

    SetUpdated();
}

void CCamera::SetupOrtograpicProjectionMatrixRH(float Width, float Height, float NearPlane, float FarPlane)
{
    m_Data.Width = Width;
    m_Data.Height = Height;
    m_Data.NearPlane = NearPlane;
    m_Data.FarPlane = FarPlane;

    // note that we use inverted z so 1 is near and 0 is far, and farPlane and nearPlane intended inverted in this call
    m_Data.ProjectionMat = Mat44::CreateOrthographic(Width, Height, FarPlane, NearPlane);

    SetUpdated();
}

void CCamera::SetupOrtograpicOffCenterProjectionMatrixRH(float Left, float Right, float Bottom, float Top, float NearPlane, float FarPlane)
{
    m_Data.Width = Right - Left;
    m_Data.Height = Top - Bottom;
    m_Data.NearPlane = NearPlane;
    m_Data.FarPlane = FarPlane;

    // note that we use inverted z so 1 is near and 0 is far, and farPlane and nearPlane intended inverted in this call
    m_Data.ProjectionMat = Mat44::CreateOrthographicOffCenter(Left, Right, Bottom, Top, FarPlane, NearPlane);

    SetUpdated();
}

void CCamera::Update()
{
    UpdatePrevFrameData();

    m_IsUpdated = false;
}

void CCamera::SetJitter(Vec2 jitter)
{
    // #todo_camera refactor
    m_PrevFrameData.Jitter = m_Data.Jitter;

    m_Data.Jitter = jitter;
}

Mat44 CCamera::GetJitteredProjectionMat(uint32 renderTargetWidth, uint32 renderTargetHeight) const
{
    Mat44 output = m_Data.ProjectionMat;

    CASSERT(false); // #todo_camera
    //output.m[2][0] = m_Data.Jitter.X / float(renderTargetWidth);
    //output.m[2][1] = m_Data.Jitter.Y / float(renderTargetHeight);

    return output;
}

float CCamera::GetEV100() const
{
    // EV number is defined as:
    // 2^ EV_s = N^2 / t and EV_s = EV_100 + log2 (S /100)
    // This gives
    // EV_s = log2 (N^2 / t)
    // EV_100 + log2 (S /100) = log2 (N^2 / t)
    // EV_100 = log2 (N^2 / t) - log2 (S /100)
    // EV_100 = log2 (N^2 / t . 100 / S)
    return log2f((m_Data.Aperture * m_Data.Aperture) / m_Data.ShutterSpeed * 100.f / m_Data.Iso);
}

void CCamera::UpdatePrevFrameData()
{
    m_PrevFrameData = m_Data;

    // remove jittering
    CASSERT(false); // #todo_camera
    //m_PrevFrameData.ProjectionMat.m[2][0] = 0.f;
    //m_PrevFrameData.ProjectionMat.m[2][1] = 0.f;
}

}
