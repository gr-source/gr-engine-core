#pragma once

#include "gr-base/core/rendertarget.hpp"

#include <gr-math/gmath.hpp>

struct Camera
{
    RenderTarget* rendertarget;

    Matrix4x4 projection;

    float nearPlane;
    float farPlane;
    float aspect;
    float fov;

    Rect rect;
};







