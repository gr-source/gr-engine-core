#pragma once

#include <cstddef>
#include <cstdint>

#include <gr-render/gCommon.h>

#include "gr-base/core/file/default.hpp"

#include "gr-base/script/default.hpp"

class Skybox
{
public:
    Skybox();

    void set_texture(grr::gTexture *texture);

    void set_skyshader(grr::gShader *shader);

    void build(grr::gTexture *texture);

    void render(const Camera *camera) const;

    bool serialize(Stream *stream) const;

    bool deserialize(Stream *stream);

private:
    grr::gTexture *m_cubemapTexture;

    std::uint32_t m_fbo;

    std::uint32_t m_rbo;

    std::size_t m_count;

    std::uint32_t m_vbo;

    std::uint32_t m_ebo;

    grr::gVertexArray *m_vertexarray;

    grr::gShader *m_skyshader;

    void upload_cubemap_texture(grr::gTexture *texture, grr::gShader *shader);

    void create_cube_mesh();

    static std::uint32_t create_renderbuffer();
};



