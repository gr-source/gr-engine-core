#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <vector>

class RenderSystem : public ISystemImpl<Read<TransformWorld, MeshFilter, MeshRender>, Write<>>
{
public:
    RenderSystem() = default;
    ~RenderSystem() override = default;

private:
    std::vector<Entity> visibleEntities;
    std::vector<Entity> entities;

    void Update() override;

    void Render() override;

    void SyncBVHBounds();
};

namespace engine
{
template <>
struct category_traits<RenderSystem>
{
    static constexpr TypeCategory value =
        TypeCategory::System;
};
}

