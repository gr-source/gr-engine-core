#include "gr-base/core/culling/frustum.hpp"

#include <cfloat>

enum FrustumSide {
	RIGHT	= 0,		// The RIGHT side of the frustum
	LEFT	= 1,		// The LEFT	 side of the frustum
	BOTTOM	= 2,		// The BOTTOM side of the frustum
	TOP		= 3,		// The TOP side of the frustum
	BACK	= 4,		// The BACK	side of the frustum
	FRONT	= 5			// The FRONT side of the frustum
}; 

enum PlaneData {
	A = 0,				// The X value of the plane's normal
	B = 1,				// The Y value of the plane's normal
	C = 2,				// The Z value of the plane's normal
	D = 3				// The distance the plane is from the origin
};

void NormalizePlane(float frustum[6][4], int side) {
	float magnitude = std::sqrt(frustum[side][A] * frustum[side][A] +  frustum[side][B] * frustum[side][B] + frustum[side][C] * frustum[side][C]);
	
	if (magnitude > 1e-5f) {
        float invMagnitude = 1.0f / magnitude;
        frustum[side][A] *= invMagnitude;
        frustum[side][B] *= invMagnitude;
        frustum[side][C] *= invMagnitude;
        frustum[side][D] *= invMagnitude;
    }
}

float Frustum::m_Frustum[6][4] {
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 0.0f},
};

void Frustum::CalculateFrustum(const Matrix4x4& vp) 
{
	auto data = reinterpret_cast<const float *>(&vp);

	// This will extract the RIGHT side of the frustum
	m_Frustum[RIGHT][A] = data[ 3] - data[ 0];
	m_Frustum[RIGHT][B] = data[ 7] - data[ 4];
	m_Frustum[RIGHT][C] = data[11] - data[ 8];
	m_Frustum[RIGHT][D] = data[15] - data[12];

	NormalizePlane(m_Frustum, RIGHT);

	// This will extract the LEFT side of the frustum
	m_Frustum[LEFT][A] = data[ 3] + data[ 0];
	m_Frustum[LEFT][B] = data[ 7] + data[ 4];
	m_Frustum[LEFT][C] = data[11] + data[ 8];
	m_Frustum[LEFT][D] = data[15] + data[12];

	// Normalize the LEFT side
	NormalizePlane(m_Frustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	m_Frustum[BOTTOM][A] = data[ 3] + data[ 1];
	m_Frustum[BOTTOM][B] = data[ 7] + data[ 5];
	m_Frustum[BOTTOM][C] = data[11] + data[ 9];
	m_Frustum[BOTTOM][D] = data[15] + data[13];

	// Normalize the BOTTOM side
	NormalizePlane(m_Frustum, BOTTOM);

	// This will extract the TOP side of the frustum
	m_Frustum[TOP][A] = data[ 3] - data[ 1];
	m_Frustum[TOP][B] = data[ 7] - data[ 5];
	m_Frustum[TOP][C] = data[11] - data[ 9];
	m_Frustum[TOP][D] = data[15] - data[13];

	// Normalize the TOP side
	NormalizePlane(m_Frustum, TOP);

	// This will extract the BACK side of the frustum
	m_Frustum[BACK][A] = data[ 3] - data[ 2];
	m_Frustum[BACK][B] = data[ 7] - data[ 6];
	m_Frustum[BACK][C] = data[11] - data[10];
	m_Frustum[BACK][D] = data[15] - data[14];

	// Normalize the BACK side
	NormalizePlane(m_Frustum, BACK);

	// This will extract the FRONT side of the frustum
	m_Frustum[FRONT][A] = data[ 3] + data[ 2];
	m_Frustum[FRONT][B] = data[ 7] + data[ 6];
	m_Frustum[FRONT][C] = data[11] + data[10];
	m_Frustum[FRONT][D] = data[15] + data[14];

	// Normalize the FRONT side
	NormalizePlane(m_Frustum, FRONT);
}

bool Frustum::IsPointInFrustum(const Vector3 &point, int planeIndex) {
    return m_Frustum[planeIndex][A] * point.x +
           m_Frustum[planeIndex][B] * point.y +
           m_Frustum[planeIndex][C] * point.z +
           m_Frustum[planeIndex][D] > 0.0f;
}

bool Frustum::IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model)
{
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
    return true;
}

// max 170
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

bool Frustum::IsSphereInFrustum(const Vector3 &center, float radius)
{
    for (int i = 0; i < 6; i++) {
        float distance = m_Frustum[i][A] * center.x +
                         m_Frustum[i][B] * center.y +
                         m_Frustum[i][C] * center.z +
                         m_Frustum[i][D];
        if (distance < -radius)
            return false;
    }
    return true;
}

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
                if (IntersectPlanes(m_Frustum[i], m_Frustum[j], m_Frustum[k], point))
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


