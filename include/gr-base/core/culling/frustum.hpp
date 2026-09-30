#pragma once

#include <gr-math/aabb.hpp>
#include <gr-math/gmath.hpp>

enum class FrustumResult
{
    Outside,
    Intersect,
    Inside
};

namespace Frustum
{
    void CalculateFrustum(const Matrix4x4& matrix);

    FrustumResult TestAABB(const Vector3& min, const Vector3& max, int planeIndex);

    // static bool IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model);

    bool IsCubeInFrustumAABB(const Vector3& min, const Vector3& max);

    bool IsSphereInFrustum(const Vector3& center, float radius);

    // static AABB ComputeAABB();

    // static float m_Frustum[6][4];

    // static bool IntersectPlanes(const float planeA[4], const float planeB[4], const float planeC[4], Vector3 &out);
};
