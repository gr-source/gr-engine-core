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

    void set_texture(gr::gTexture *texture);

    void set_skyshader(gr::Shader *shader);

    void build(gr::gTexture *texture);

    void render(const Camera *camera) const;

    bool serialize(Stream *stream) const;

    bool deserialize(Stream *stream);

private:
    gr::gTexture *m_cubemapTexture;

    std::uint32_t m_fbo;

    std::uint32_t m_rbo;

    std::size_t m_count;

    std::uint32_t m_vbo;

    std::uint32_t m_ebo;

    gr::gVertexArray *m_vertexarray;

    gr::Shader *m_skyshader;

    void upload_cubemap_texture(gr::gTexture *texture, gr::Shader *shader);

    void create_cube_mesh();

    static std::uint32_t create_renderbuffer();
};



