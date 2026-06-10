#pragma once

#include "gr-base/core/renderer.hpp"

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

struct CameraData
{
    Matrix4x4 projection      = Matrix4x4::identityMatrix;
    Matrix4x4 view            = Matrix4x4::identityMatrix;
    Matrix4x4 viewProjection  = Matrix4x4::identityMatrix;

    Vector3 forward           = Vector3::forward;
    Vector3 right             = Vector3::right;
    Vector3 up                = Vector3::up;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraData> =
    TypeCategory::Component;





