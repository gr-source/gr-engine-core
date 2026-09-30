#include "gr-base/core/culling/frustum.hpp"

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

static Plane gPlanes[6];

void Frustum::CalculateFrustum(const Matrix4x4& matrix) 
{
    const Matrix4x4 vp = Math::transpose(matrix);

    const Vector4& column0 = vp[0];
    const Vector4& column1 = vp[1];
    const Vector4& column2 = vp[2];
    const Vector4& column3 = vp[3];

    gPlanes[0].col = column3 + column0;
    gPlanes[1].col = column3 - column0;
    gPlanes[2].col = column3 + column1;
    gPlanes[3].col = column3 - column1;
    gPlanes[4].col = column3 + column2;
    gPlanes[5].col = column3 - column2;

    for (int i=0;i<6;i++)
    {
        auto& p = gPlanes[i];

        float len = Math::length(p.normal);

        p.normal /= len;
        p.d /= len;
    }
}

FrustumResult Frustum::TestAABB(const Vector3& min, const Vector3& max, int planeIndex)
{
    const auto& p = gPlanes[planeIndex];

    Vector3 positive;
    Vector3 negative;

    positive.x = (p.normal.x >= 0.0f) ? max.x : min.x;
    positive.y = (p.normal.y >= 0.0f) ? max.y : min.y;
    positive.z = (p.normal.z >= 0.0f) ? max.z : min.z;

    negative.x = (p.normal.x >= 0.0f) ? min.x : max.x;
    negative.y = (p.normal.y >= 0.0f) ? min.y : max.y;
    negative.z = (p.normal.z >= 0.0f) ? min.z : max.z;

    float positiveDistance =
        Math::dot(p.normal, positive) + p.d;

    float negativeDistance =
        Math::dot(p.normal, negative) + p.d;

    if (positiveDistance < 0.0f)
        return FrustumResult::Outside;

    if (negativeDistance >= 0.0f)
        return FrustumResult::Inside;

    return FrustumResult::Intersect;
}

// bool Frustum::IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model)
// {
    /*
    Vector3 points[8] {
        Vector3(model * Vector4{min.x, min.y, min.z, 1.0f}),
        Vector3(model * Vector4{max.x, min.y, min.z, 1.0f}),
        Vector3(model * Vector4{max.x, max.y, min.z, 1.0f}),
        Vector3(model * Vector4{min.x, max.y, min.z, 1.0f}),

        Vector3(model * Vector4{min.x, min.y, max.z, 1.0f}),
        Vector3(model * Vector4{max.x, min.y, max.z, 1.0f}),
        Vector3(model * Vector4{max.x, max.y, max.z, 1.0f}),
        Vector3(model * Vector4{min.x, max.y, max.z, 1.0f})
    };
    */

    /*
	if (DebugDraw::bDebugMode) {
        DrawCube(points);
    }
	*/

    /*
    for (int i = 0; i < 6; i++) {
        bool allPointsOutside = true;
        for (int j = 0; j < 8; j++) {
            if (IsPointInFrustum(points[j], i)) {
                allPointsOutside = false;
                break;
            }
        }
        if (allPointsOutside) return false;
    }
    */
//     return true;
// }

bool Frustum::IsCubeInFrustumAABB(const Vector3& min,
                                  const Vector3& max)
{
    for (int i = 0; i < 6; ++i)
    {
        const auto& p = gPlanes[i];

        Vector3 positive;

        positive.x = (p.normal.x >= 0.0f) ? max.x : min.x;
        positive.y = (p.normal.y >= 0.0f) ? max.y : min.y;
        positive.z = (p.normal.z >= 0.0f) ? max.z : min.z;

        if (Math::dot(p.normal, positive) + p.d < 0.0f)
            return false;
    }

    return true;
}

/*
bool Frustum::IsCubeInFrustumAABB(const Vector3& min, const Vector3& max) {
    Vector3 points[8] {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {max.x, max.y, min.z},
        {min.x, max.y, min.z},
 
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z},
        {min.x, max.y, max.z}
    };

    // DrawCube(points);

	for (int i=0;i<6;i++) {
        bool allPointsOutside = true;
        for (int j=0;j<8;j++) {
            if (IsPointInFrustum(points[j], i)) {
                allPointsOutside = false;
                break;
            }
        }
        if (allPointsOutside) return false;
    }
	return true;
}
*/

bool Frustum::IsSphereInFrustum(const Vector3 &center, float radius)
{
    for (int i = 0; i < 6; i++) {
        float distance = gPlanes[i][0] * center.x +
                         gPlanes[i][1] * center.y +
                         gPlanes[i][2] * center.z +
                         gPlanes[i][3];
        if (distance < -radius)
            return false;
    }
    return true;
}

/*
AABB Frustum::ComputeAABB()
{
    Vector3 min{+FLT_MAX, +FLT_MAX, +FLT_MAX};
    Vector3 max{-FLT_MAX, -FLT_MAX, -FLT_MAX};

    // Para cada combinação de 3 planos, encontramos um vértice do frustum
    for (int i = 0; i < 6; i++)
    {
        for (int j = i + 1; j < 6; j++)
        {
            for (int k = j + 1; k < 6; k++)
            {
                Vector3 point;
                if (IntersectgPlanes(gPlanes[i], gPlanes[j], gPlanes[k], point))
                {
                    min.x = std::min(min.x, point.x);
                    min.y = std::min(min.y, point.y);
                    min.z = std::min(min.z, point.z);

                    max.x = std::max(max.x, point.x);
                    max.y = std::max(max.y, point.y);
                    max.z = std::max(max.z, point.z);
                }
            }
        }
    }

    return AABB{min, max};
}
*/

/*
bool Frustum::IntersectgPlanes(const float planeA[4], const float planeB[4], const float planeC[4], Vector3 &out)
{
    // Usando determinante
    Vector3 n1{planeA[0], planeA[1], planeA[2]};
    Vector3 n2{planeB[0], planeB[1], planeB[2]};
    Vector3 n3{planeC[0], planeC[1], planeC[2]};

    float det = Math::dot(n1, Math::cross(n2, n3));
    if (fabs(det) < 1e-6f)
        return false;

    Vector3 p = -planeA[3] * Math::cross(n2, n3) - planeB[3] * Math::cross(n3, n1) - planeC[3] * Math::cross(n1, n2) / det;
    out = p;
    return true;
}
*/

