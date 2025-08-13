#include "gr-base/core/culling/frustum.hpp"

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

void Frustum::CalculateFrustum(const Matrix4x4& vp) {   
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

	/*
	if (DebugDraw::bDebugMode) {
        DrawCube(points);
    }
	*/

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

void Frustum::DrawCube(Vector3 points[8]) {
	/*
	DebugDraw::AddLine(points[0], points[1], Color::blue);
	DebugDraw::AddLine(points[1], points[2], Color::blue);
	DebugDraw::AddLine(points[2], points[3], Color::blue);
	DebugDraw::AddLine(points[3], points[0], Color::blue);

	DebugDraw::AddLine(points[4], points[5], Color::blue);
	DebugDraw::AddLine(points[5], points[6], Color::blue);
	DebugDraw::AddLine(points[6], points[7], Color::blue);
	DebugDraw::AddLine(points[7], points[4], Color::blue);

	DebugDraw::AddLine(points[0], points[4], Color::blue);
	DebugDraw::AddLine(points[1], points[5], Color::blue);
	DebugDraw::AddLine(points[2], points[6], Color::blue);
	DebugDraw::AddLine(points[3], points[7], Color::blue);
	*/
}

/*
const bool Frustum::PointInFrustum(const Vector3& point) const {
	for (int i = 0; i < 6; i++) {
		if (m_Frustum[i][A] * point.x + m_Frustum[i][B] * point.y + m_Frustum[i][C] * point.z + m_Frustum[i][D] <= 0) {
			return false;
		}
	}
	return true;
}

const bool Frustum::SphereInFrustum(const Vector3& point, float radius) const {
	for (int i = 0; i < 6; i++)	{
		if (m_Frustum[i][A] * point.x + m_Frustum[i][B] * point.y + m_Frustum[i][C] * point.z + m_Frustum[i][D] <= -radius) {
			return false;
		}
	}
	return true;
}

const bool Frustum::IsCubeInFrustum(const Vector3 *vertice) const {
    for (int i = 0; i < 6; i++ ) {
		if (m_Frustum[i][A] * (vertice[0].x) + m_Frustum[i][B] * (vertice[0].y) + m_Frustum[i][C] * (vertice[0].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[1].x) + m_Frustum[i][B] * (vertice[1].y) + m_Frustum[i][C] * (vertice[1].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[2].x) + m_Frustum[i][B] * (vertice[2].y) + m_Frustum[i][C] * (vertice[2].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[3].x) + m_Frustum[i][B] * (vertice[3].y) + m_Frustum[i][C] * (vertice[3].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[4].x) + m_Frustum[i][B] * (vertice[4].y) + m_Frustum[i][C] * (vertice[4].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[5].x) + m_Frustum[i][B] * (vertice[5].y) + m_Frustum[i][C] * (vertice[5].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[6].x) + m_Frustum[i][B] * (vertice[6].y) + m_Frustum[i][C] * (vertice[6].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (vertice[7].x) + m_Frustum[i][B] * (vertice[7].y) + m_Frustum[i][C] * (vertice[7].z) + m_Frustum[i][D] > 0)
		   continue;
		return false;
	}
	return true;
}

#include "../renderer.h"

const bool Frustum::IsCubeInFrustumAABB(const Vector3& min, const Vector3& max) const {
    Vector3 points[8] {
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(min.x, max.y, min.z),
 
        Vector3(min.x, min.y, max.z),
        Vector3(max.x, min.y, max.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)
    };

	// Desenhar as linhas do cubo usando as coordenadas dos pontos do frustum
	DebugDraw::AddLine(points[0], points[1], Color::blue);
	DebugDraw::AddLine(points[1], points[2], Color::blue);
	DebugDraw::AddLine(points[2], points[3], Color::blue);
	DebugDraw::AddLine(points[3], points[0], Color::blue);

	DebugDraw::AddLine(points[4], points[5], Color::blue);
	DebugDraw::AddLine(points[5], points[6], Color::blue);
	DebugDraw::AddLine(points[6], points[7], Color::blue);
	DebugDraw::AddLine(points[7], points[4], Color::blue);

	DebugDraw::AddLine(points[0], points[4], Color::blue);
	DebugDraw::AddLine(points[1], points[5], Color::blue);
	DebugDraw::AddLine(points[2], points[6], Color::blue);
	DebugDraw::AddLine(points[3], points[7], Color::blue);

	for (int i = 0; i < 6; i++ ) {
		if (m_Frustum[i][A] * (points[0].x) + m_Frustum[i][B] * (points[0].y) + m_Frustum[i][C] * (points[0].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[1].x) + m_Frustum[i][B] * (points[1].y) + m_Frustum[i][C] * (points[1].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[2].x) + m_Frustum[i][B] * (points[2].y) + m_Frustum[i][C] * (points[2].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[3].x) + m_Frustum[i][B] * (points[3].y) + m_Frustum[i][C] * (points[3].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[4].x) + m_Frustum[i][B] * (points[4].y) + m_Frustum[i][C] * (points[4].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[5].x) + m_Frustum[i][B] * (points[5].y) + m_Frustum[i][C] * (points[5].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[6].x) + m_Frustum[i][B] * (points[6].y) + m_Frustum[i][C] * (points[6].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[7].x) + m_Frustum[i][B] * (points[7].y) + m_Frustum[i][C] * (points[7].z) + m_Frustum[i][D] > 0)
		   continue;
		return false;
	}
	return true;
}

const bool Frustum::IsCubeInFrustumOOBB(const Vector3 &min, const Vector3 &max, const Matrix4x4 &model) const {
	Vector3 points[8] {
		Vector3(model * Vector4(min.x, min.y, min.z, 1.0f)),
		Vector3(model * Vector4(max.x, min.y, min.z, 1.0f)),
		Vector3(model * Vector4(max.x, max.y, min.z, 1.0f)),
		Vector3(model * Vector4(min.x, max.y, min.z, 1.0f)),

		Vector3(model * Vector4(min.x, min.y, max.z, 1.0f)),
		Vector3(model * Vector4(max.x, min.y, max.z, 1.0f)),
		Vector3(model * Vector4(max.x, max.y, max.z, 1.0f)),
		Vector3(model * Vector4(min.x, max.y, max.z, 1.0f))
	};

	if (DebugDraw::bDebugMode) {

		// Desenhar as linhas do cubo usando as coordenadas dos pontos do frustum
		DebugDraw::AddLine(points[0], points[1], Color::blue);
		DebugDraw::AddLine(points[1], points[2], Color::blue);
		DebugDraw::AddLine(points[2], points[3], Color::blue);
		DebugDraw::AddLine(points[3], points[0], Color::blue);

		DebugDraw::AddLine(points[4], points[5], Color::blue);
		DebugDraw::AddLine(points[5], points[6], Color::blue);
		DebugDraw::AddLine(points[6], points[7], Color::blue);
		DebugDraw::AddLine(points[7], points[4], Color::blue);

		DebugDraw::AddLine(points[0], points[4], Color::blue);
		DebugDraw::AddLine(points[1], points[5], Color::blue);
		DebugDraw::AddLine(points[2], points[6], Color::blue);
		DebugDraw::AddLine(points[3], points[7], Color::blue);
	}

	for (int i = 0; i < 6; i++ ) {
		if (m_Frustum[i][A] * (points[0].x) + m_Frustum[i][B] * (points[0].y) + m_Frustum[i][C] * (points[0].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[1].x) + m_Frustum[i][B] * (points[1].y) + m_Frustum[i][C] * (points[1].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[2].x) + m_Frustum[i][B] * (points[2].y) + m_Frustum[i][C] * (points[2].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[3].x) + m_Frustum[i][B] * (points[3].y) + m_Frustum[i][C] * (points[3].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[4].x) + m_Frustum[i][B] * (points[4].y) + m_Frustum[i][C] * (points[4].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[5].x) + m_Frustum[i][B] * (points[5].y) + m_Frustum[i][C] * (points[5].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[6].x) + m_Frustum[i][B] * (points[6].y) + m_Frustum[i][C] * (points[6].z) + m_Frustum[i][D] > 0)
		   continue;
		if (m_Frustum[i][A] * (points[7].x) + m_Frustum[i][B] * (points[7].y) + m_Frustum[i][C] * (points[7].z) + m_Frustum[i][D] > 0)
		   continue;
		return false;
	}
	return true;
}
*/
