#pragma once

#include "gr-base/core/renderer.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-math/gmath.hpp>
#include <gr-math/rect.h>

struct Camera
{
    RenderTarget* rendertarget =
        nullptr;

    Rect viewport =
        { 0.0f, 0.0f, 512.0f, 512.0f };
    float nearPlane =
        0.001f;
    float farPlane =
        500.0f;
    float fov =
        45.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Camera> =
    TypeCategory::Component;

struct CameraData
{
    Matrix4x4 projection =
        type_traits<Matrix4x4>::identity;
    Matrix4x4 view =
        type_traits<Matrix4x4>::identity;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraData> =
    TypeCategory::Component;





