#pragma once

#include <gr-math/aabb.hpp>
#include <gr-math/gmath.hpp>

class Frustum {
public:
    static void CalculateFrustum(const Matrix4x4& matrix);

    // static bool IsPointInFrustum(const Vector3& point, int planeIndex);

    // static bool IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model);

    static bool IsCubeInFrustumAABB(const Vector3& min, const Vector3& max);

    static bool IsSphereInFrustum(const Vector3& center, float radius);

    // static AABB ComputeAABB();

private:
    struct Plane
    {
        union
        {
            struct
            {
                Vector3 normal;
                float d;
            };
            Vector4 col;
        };

        float& operator [](int index) noexcept
        {
            return col[index];
        }
    };

    static_assert(sizeof(Plane) == sizeof(Vector4));

    static Plane planes[6];

    // static float m_Frustum[6][4];

    static bool IntersectPlanes(const float planeA[4], const float planeB[4], const float planeC[4], Vector3 &out);
};
