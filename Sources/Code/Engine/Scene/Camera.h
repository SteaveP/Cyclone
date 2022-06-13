#pragma once

#include "Engine/Core/Math.h"
#include "Engine/EngineModule.h"

namespace Cyclone
{

class CCamera;
using CCameraPtr = Ptr<CCamera>;

class ENGINE_API CCamera
{
public:
    struct CFrameData
    {
        // #todo add cached values like WorldViewMat?
        Mat44 WorldMat; 
        Mat44 ViewMat;
        Mat44 ProjectionMat;

        float NearPlane = 0.01f;
        float FarPlane = 1000.f;

        float vFov = 1.f;
        float AspectRatio = 1.f;

        float Width = 1; // for orthographic, in world units
        float Height = 1; // for orthographic, in world units

        // real camera parameters from Sunny 16 rule (google it)
        float Iso = 100.f;
        float ShutterSpeed = 1.f / 100.f;
        float Aperture = 1.f / 16.f;

        Vec2 Jitter;
    };

public:
    CCamera();
    ~CCamera();

    void SetWorldTransform(const Mat44& worldTransform);
    const Mat44& GetWorldTransform() const { return m_Data.WorldMat; }

    void LookAtMatrix(const Vec3& viewerPos, const Vec3& lookAtPos);

    void SetupPerspectiveProjectionMatrixRH(float vFov, float AspectRatio, float NearPlane, float FarPlane);
    void SetupOrtograpicProjectionMatrixRH(float width, float height, float nearPlane, float farPlane);
    void SetupOrtograpicOffCenterProjectionMatrixRH(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    void Update();

    bool IsUpdated() const { return m_IsUpdated; }
    void SetUpdated() { m_IsUpdated = true; }

    const CFrameData& GetPrevFrameData() const { return m_PrevFrameData; }

    void SetJitter(Vec2 jitter);
    Vec2 GetJitter() const { return m_Data.Jitter; }

    Mat44 GetJitteredProjectionMat(uint32 renderTargetWidth, uint32 renderTargetHeight) const;

    float GetEV100() const;

protected:
    void UpdatePrevFrameData();

public:
    bool m_IsUpdated = true;

    CFrameData m_Data;
    CFrameData m_PrevFrameData;
};

}
