#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/system.hpp"

#include "gr-base/managers/default.hpp"

#include <gr-render/shader.hpp>

struct Renderable
{
    uint64_t sortKey;
    MaterialHandle material;
    MeshHandle mesh;
    uint32_t submesh;
    Matrix4x4 *world;
};

class RenderSystem : public System
{
public:
    void Update(Scene* scene) override;

private:
    std::vector<Renderable> m_renderables;

    std::vector<Matrix4x4> m_metrices;

    void sorting();

    static void Setuplight(gr::Shader* shader);
};


