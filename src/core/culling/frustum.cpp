#include "gr-base/core/culling/frustum.hpp"

#include <cfloat>

enum FrustumSide
{
	RIGHT	= 0,
	LEFT	= 1,
	BOTTOM	= 2,
	TOP		= 3,
    BACK	= 4,
    FRONT	= 5
}; 

Frustum::Plane Frustum::planes[6] {};

void Frustum::CalculateFrustum(const Matrix4x4& vp) 
{
    Vector4 row0{
        vp[0].x,
        vp[1].x,
        vp[2].x,
        vp[3].x
    };

    Vector4 row1{
        vp[0].y,
        vp[1].y,
        vp[2].y,
        vp[3].y
    };

    Vector4 row2{
        vp[0].z,
        vp[1].z,
        vp[2].z,
        vp[3].z
    };

    Vector4 row3{
        vp[0].w,
        vp[1].w,
        vp[2].w,
        vp[3].w
    };

    planes[LEFT].col   = row3 + row0;
    planes[RIGHT].col  = row3 - row0;

    planes[BOTTOM].col = row3 + row1;
    planes[TOP].col    = row3 - row1;

    planes[FRONT].col  = row3 + row2;
    planes[BACK].col   = row3 - row2;

    for (int i=0;i<6;i++)
    {
        auto& p = planes[i];

        float len = Math::magnitude(p.normal);

        p.normal /= len;
        p.d /= len;
    }
}

// bool Frustum::IsPointInFrustum(const Vector3 &point, int planeIndex) {
//     return planes[planeIndex][A] * point.x +
//            planes[planeIndex][B] * point.y +
//            planes[planeIndex][C] * point.z +
//            planes[planeIndex][D] > 0.0f;
// }

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
        const auto& p = planes[i];

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
        float distance = planes[i][0] * center.x +
                         planes[i][1] * center.y +
                         planes[i][2] * center.z +
                         planes[i][3];
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
                if (IntersectPlanes(planes[i], planes[j], planes[k], point))
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

bool Frustum::IntersectPlanes(const float planeA[4], const float planeB[4], const float planeC[4], Vector3 &out)
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


