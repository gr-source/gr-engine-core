#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/script/camera.hpp"
#include "gr-base/script/default.hpp"

#include <gr-math/gmath.hpp>

class CameraSystem : public ISystemImpl<Read<TransformWorld, Camera>, Write<CameraData>>
{
public:
    void Update() override;

    Matrix4x4 CreateViewMatrix(const TransformWorld* Transform) const;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraSystem> =
    TypeCategory::System;


