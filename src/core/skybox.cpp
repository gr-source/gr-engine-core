#include "gr-base/core/skybox.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/file/stream.hpp"
// #include "gr-base/core/geometry/shader.hpp"
#include "gr-base/script/camera.hpp"

#include <cstdint>
#include <gr-render/gCommon.h>
#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/shader.hpp>

#include <gr-render/gl.h>
#include <string>

Skybox::Skybox() : m_cubemapTexture(HANDLE_INVALID), m_fbo(0), m_rbo(0), m_count(0), m_vbo(0), m_ebo(0), m_skyshader(nullptr)
{}

void Skybox::set_texture(gr::gTexture *texture)
{
    build(texture);
}

void Skybox::set_skyshader(gr::Shader *shader)
{
    m_skyshader = shader;
}

void Skybox::build(gr::gTexture *texture)
{
    // configure texture cubemap
    m_cubemapTexture = AssetsManager::CreateTexture();
    // m_cubemapTexture->set_texture(gTextureFlags_Cubemap);
    // m_cubemapTexture->set_clamping(gTextureFlags_Clamp_Edge);
    // m_cubemapTexture->set_filtering(gTextureFlags_Filter_Linear);

    gTextureCubemapFace_ faceid[6] = {
        gTextureCubemapFace_Front, gTextureCubemapFace_Back,
        gTextureCubemapFace_Top, gTextureCubemapFace_Bottom,
        gTextureCubemapFace_Left, gTextureCubemapFace_Right
    };

    for (int i=0;i<6;i++)
    {
        // m_cubemapTexture->set_face(faceid[i]);
        // m_cubemapTexture->updateBuffer(512, 512, nullptr);
    }

    // configure rbo
    m_rbo = create_renderbuffer();

    // configure fbo and linker texture and rbo
    {
        glGenFramebuffers(1, &m_fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Erro ao configurar o framebuffer!" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    create_cube_mesh();

    /*
    ShaderParams shaderParams;
    shaderParams.name = "skybox";
    shaderParams.base = gfile::get_data_directory() + "/assets/shader/skybox/skybox.glsl";
    shaderParams.flags = ShaderOptimize_static;
    shaderParams.type = ShaderType::SKY;
    
    AssetsManager::LoadShaderAsync(
        shaderParams,
        &Skybox::upload_cubemap_texture,
        this,
        texture
    );
    */
}

void Skybox::render(const Camera *camera) const
{
    /*
    if (m_skyshader == nullptr)
        return;

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();

    m_cubemapTexture->bind(0);
    m_skyshader->set_uniform("u_texture", 0);
    
    m_skyshader->setUniform("u_projection", &projection);
    m_skyshader->setUniform("u_view", &view);

    m_skyshader->bind();

    m_vertexarray->bind();
    
    gr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_count, nullptr);

    m_vertexarray->unbind();
    m_skyshader->unbind();
    m_cubemapTexture->unbind();
    */
}

bool Skybox::serialize(Stream *stream) const
{
    return true;
}

bool Skybox::deserialize(Stream *stream)
{
    return true;
}

// ********** private ********** //
void Skybox::upload_cubemap_texture(gr::gTexture *texture, gr::Shader *shader)
{
    auto projection = Math::perspective(Math::rad(90.0f), 1.0f, 0.1f, 10.0f);

    // cubefraembuffer a ta usando id 0
    texture->bind(0);
    shader->set_uniform("u_texture", 0);

    shader->setUniform("u_projection", &projection);

    Matrix4x4 view[6] {
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f,  0.0f,  1.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f,  0.0f, -1.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, -1.0f,  0.0f}),
        Math::lookAt({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, -1.0f,  0.0f})
    };

    shader->bind();

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, 512, 512);

    for (auto i=0;i<6;i++) {
        shader->setUniform("u_view", &view[i]);

        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_cubemapTexture->getTextureID(), 0);

        gr::gRender::SetBackgroundColor(COLOR_BLUE);
        gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_COLOR_BUFFER | gr::GR_DEPTH_BUFFER);
        
        m_vertexarray->bind();
        
        gr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_count, nullptr);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    texture->unbind();
    shader->unbind();

    /*
    ShaderParams shaderParams;
    shaderParams.name = "background";
    shaderParams.base = gfile::get_data_directory() + "/assets/shader/skybox/background.glsl";
    shaderParams.flags = ShaderOptimize_static;
    shaderParams.type = ShaderType::SKY;

    AssetsManager::LoadShaderAsync(shaderParams,
            &Skybox::set_skyshader, this
            );
    */
}

void Skybox::create_cube_mesh()
{
    std::vector<Vector3> vertices = {
        { 1.0f,  1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f}
    };

    std::vector<u32> indices = {
        1, 0, 4,
        5, 1, 4,
        1, 3, 0,
        3, 2, 0,
        3, 7, 2,
        7, 6, 2,
        0, 2, 4,
        2, 6, 4,
        5, 7, 1,
        7, 3, 1,
        7, 5, 6,
        5, 4, 6
    };

    m_count = indices.size();

    m_vbo = gr::gVertexArray::CreateBuffer(BufferType::VBO, vertices.data(), vertices.size() * sizeof(Vector3));
    assert(m_vbo && "Falied to create vbo.");

    m_ebo = gr::gVertexArray::CreateBuffer(BufferType::EBO, indices.data(), indices.size() * sizeof(std::uint32_t));
    assert(m_vbo && "Falied to create ebo.");

    m_vertexarray = new gr::gVertexArray();
    assert(m_vertexarray && "Falied to create vao.");

    m_vertexarray->bind();

    gr::gVertexArray::Bind(m_ebo);

    gr::gVertexArray::Bind(m_vbo);
    gr::gVertexArray::SetAttrib(0, 3, sizeof(Vector3), nullptr);

    m_vertexarray->unbind();
}

std::uint32_t Skybox::create_renderbuffer()
{
    std::uint32_t renderbufferID = 0;

    glGenRenderbuffers(1, &renderbufferID);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return renderbufferID;
}


