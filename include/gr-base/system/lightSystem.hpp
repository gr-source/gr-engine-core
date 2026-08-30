#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/light.hpp"

class LightSystem : public ISystemImpl<Read<TransformLocal, Light>, Write<>>
{
public:
    LightSystem();

private:
    void Update() override;
};

namespace engine
{
template <>
struct category_traits<LightSystem>
{
    static constexpr TypeCategory value =
        TypeCategory::System;
};
}



