#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include <gr-math/gmath.hpp>

class CameraSystem : public ISystem
{
public:
    void Update() override;

    Matrix4x4 CreateViewMatrix(const Transform* Transform) const;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraSystem> =
    TypeCategory::System;


