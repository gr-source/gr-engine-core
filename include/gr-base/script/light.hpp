#pragma once

#include <gr-math/gmath.hpp>

struct Light
{
    enum LightType : int {
        eDirection = 0,
        ePoint     = 1,
        eSpot      = 2,
    };

    static Matrix4x4 mProjection;

    int m_type;

    Color m_color;

    float m_attenuationConstant;
    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;
};


