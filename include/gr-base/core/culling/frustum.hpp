#pragma once

#include <gr-math/gmath.hpp>

#include <array>

class Frustum {
public:
    static void CalculateFrustum(const Matrix4x4& vp);

    static bool IsPointInFrustum(const Vector3& point, int planeIndex);

    static bool IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model);

    static bool IsCubeInFrustumAABB(const Vector3& min, const Vector3& max);

private:
    static float m_Frustum[6][4];

    static void DrawCube(Vector3 points[8]);
};