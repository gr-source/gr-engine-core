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
struct type_traits<Camera>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(Camera);
};

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
struct type_traits<CameraData>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(CameraData);
};





