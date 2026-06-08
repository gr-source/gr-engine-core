#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/managers/default.hpp"
#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"

#include "gr-base/spatial/bvhtree.hpp"

#include <algorithm>

void RenderSystem::Update(Scene* scene)
{
    Entity cameraEntity = scene->GetCamera();
    if (cameraEntity == INVALID_ENTITY)
        return;

    auto world = scene->GetWorld();
    auto bvh = scene->GetBVHtree();

    Transform* cameraTransform =
        world->GetComponent<Transform>(cameraEntity);

    Camera* camera =
        world->GetComponent<Camera>(cameraEntity);

    CameraData* cameraData =
        world->GetComponent<CameraData>(cameraEntity);

    Renderer::SetRenderTarget(
        camera->rendertarget
    );

    Frustum::CalculateFrustum(cameraData->viewProjection);

    for (auto prefab : AssetsManager::GetAllShader3D())
    {
        for (auto shader : prefab->GetShaders())
        {
            shader->bind();
            shader->setUniform("u_mat_projection", &cameraData->projection);
            shader->setUniform("u_mat_view",       &cameraData->view);
            shader->setUniform("u_mat_eye",        &cameraTransform->position);

            shader->set_uniform("u_mat_numLights", 0);

            Setuplight(world, shader);

            shader->unbind();
        }
    }

    std::vector<Entity> entities;
    bvh->queryFrustumAABB(entities);

    for (auto& entity : entities)
    {
        auto transform = world->GetComponent<Transform>(entity);
        auto meshFilter = world->GetComponent<MeshFilter>(entity);
        auto meshRender = world->GetComponent<MeshRender>(entity);

        if (meshRender == nullptr)
            continue;

        auto& mesh = meshFilter->mesh;
        for (size_t j=0;j<meshRender->numMaterials;j++)
        {
            auto& material = meshRender->materials[j];

            uint64_t sortKey =
                  (uint64_t)handle_get_first(material) << 32
                | (uint64_t)handle_get_first(mesh) << 16
                | (uint64_t)j;

            m_renderables.push_back({
                        sortKey,
                        material,
                        mesh,
                        static_cast<uint32_t>(j),
                        &transform->worldMatrix
                    });
        }
    }

    if (m_renderables.size() >= m_metrices.size())
        m_metrices.resize(m_renderables.size() + 1, Matrix4x4::identityMatrix);

    sorting();

    m_renderables.clear();
}

void RenderSystem::sorting()
{
    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        return a.sortKey < b.sortKey;
    });

    size_t i = 0, matrixID = 0;
    while (i < m_renderables.size())
    {
        const auto& first = m_renderables[i];

        size_t start = i;

        while (start < m_renderables.size())
        {
            const auto& current = m_renderables[start];

            bool sameBatch = current.sortKey == first.sortKey;

            if (!sameBatch)
                break;

            m_metrices[matrixID++] = *current.world;

            ++start;
        }

        size_t count = start - i;

        RenderCommand cmd;
        cmd.material = AssetsManager::GetMaterial(first.material);
        cmd.mesh = AssetsManager::GetMesh(first.mesh);
        cmd.submesh = first.submesh;
        cmd.instances = &m_metrices[matrixID - count];
        cmd.instanceCount = count;

        Renderer::Submit(cmd);

        i = start;
    }
}

void RenderSystem::Setuplight(World* world, gr::Shader* shader)
{
    int numLight = 0;
    world->Query<Transform, Light>([&](Entity*, Transform* transforms, Light* lights, size_t count)
    {
        numLight += static_cast<int>(count);

        for (size_t i=0;i<count;i++)
        {
            Transform& transform = transforms[i];
            Light& light = lights[i];

            auto position   = transform.position;
            auto forward    = transform.rotation * Vector3::forward;
            auto color      = light.color;
            auto type       = light.type;

            // auto attenuation = Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()};
            // auto params      = Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()};
            // auto projection  = light->getProjetion();
            
            shader->setUniform(("u_mat_lights[" + std::to_string(i) + "].position").c_str(),    &position);
            shader->setUniform(("u_mat_lights[" + std::to_string(i) + "].direction").c_str(),   &forward);
            shader->setUniform(("u_mat_lights[" + std::to_string(i) + "].color").c_str(),       &color);
            // shader->setUniform(("lights[" + std::to_string(i) + "].attenuation").c_str(), &attenuation);
            // shader->setUniform(("lights[" + std::to_string(i) + "].params").c_str(),      &params);
            shader->setUniform(("u_mat_lights[" + std::to_string(i) + "].type").c_str(),       &type);
        }
    });

    shader->set_uniform("u_mat_numLights", numLight);
}


