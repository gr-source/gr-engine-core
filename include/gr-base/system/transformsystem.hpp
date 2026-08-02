#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/script/transform.hpp"

#include <vector>

class TransformSystem : public ISystemImpl<Read<TransformLocal>, Write<TransformWorld>>
{
private:
    std::vector<Entity> entities;

    void Update() override;

    bool SyncTransform(const GameObject& gameObject, const Matrix4x4& parentMatrix) const;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<TransformSystem> =
    TypeCategory::System;


