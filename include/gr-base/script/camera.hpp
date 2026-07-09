#pragma once

#include "gr-base/core/renderer.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-math/gmath.hpp>

struct Camera
{
    RenderTarget* rendertarget  = nullptr;

    float nearPlane             = 0.001f;
    float farPlane              = 500.0f;
    float fov                   = 45.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Camera> =
    TypeCategory::Component;

struct alignas(16) CameraData
{
    Matrix4x4 projection      = type_traits<Matrix4x4>::identity;
    Matrix4x4 view            = type_traits<Matrix4x4>::identity;
    Matrix4x4 viewProjection  = type_traits<Matrix4x4>::identity;

    Vector3 forward           = type_traits<Vector3>::forward;
    Vector3 right             = type_traits<Vector3>::right;
    Vector3 up                = type_traits<Vector3>::up;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraData> =
    TypeCategory::Component;





