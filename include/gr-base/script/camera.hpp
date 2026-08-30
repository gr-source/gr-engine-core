#pragma once

#include "gr-base/render/pipeline/pipelineID.hpp"

#include "gr-base/core/default.hpp"


#include "gr-base/ecs/type_traits.hpp"

#include <gr-render/default.hpp>
#include <gr-render/rect.hpp>

#include <gr-math/gmath.hpp>

struct Camera
{
    PipelineID pipeline =
        PipelineForward;

    Rect viewport =
        { 0.0f, 0.0f, 512.0f, 512.0f };
    float nearPlane =
        0.001f;
    float farPlane =
        500.0f;
    float fov =
        45.0f;
};

namespace engine
{
template <>
struct category_traits<Camera>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct CameraData
{
    Matrix4x4 projection =
        type_traits<Matrix4x4>::identity;
    Matrix4x4 view =
        type_traits<Matrix4x4>::identity;
};

namespace engine
{
template <>
struct category_traits<CameraData>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct CameraRenderResult
{
    TextureID color;
    TextureID depth;
};

namespace engine
{
template <>
struct category_traits<CameraRenderResult>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}




