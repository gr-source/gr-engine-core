#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/view.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/managers/default.hpp"
#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/spatial/bvhtree.hpp"

#include <algorithm>

void RenderSystem::Update(Scene* scene)
{
    Entity cameraEntity = scene->GetCamera();
    if (cameraEntity == INVALID_ENTITY)
        return;

    auto world = scene->GetWorld();

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
            // Setuplight(scene, shader);
            shader->unbind();
        }
    }

    std::vector<Entity> entities;

    auto bvh = scene->GetBVHtree();
    bvh->queryFrustumAABB(entities);

    for (auto& entity : entities)
    {
        auto transform = world->GetComponent<Transform>(entity);
        auto meshFilter = world->GetComponent<MeshFilter>(entity);
        auto meshRender = world->GetComponent<MeshRender>(entity);

        if (meshFilter == nullptr || meshRender == nullptr)
            continue;

        auto& mesh = meshFilter->mesh;
        for (size_t j=0;j<meshRender->numMaterials;j++)
        {
            auto& material = meshRender->materials[j];

            m_renderables.push_back({
                        material,
                        mesh,
                        static_cast<uint32_t>(j),
                        &transform->worldMatrix
                    });
        }
    }

    /*
    Signature requiredMask =
        Type::GetMask<Transform, MeshRender, MeshFilter>();

    auto array = scene->GetWorld()->GetRawArchetype();
    for (size_t a = 0; a < scene->GetWorld()->GetArchetypeCount(); a++)
    {
        ArchetypeStorage& archetype = array[a];

        if ((archetype.m_mask & requiredMask) != requiredMask)
            continue;

        const auto transforms  = archetype.GetRaw<Transform>();
        const auto meshRenders = archetype.GetRaw<MeshRender>();
        const auto meshFilters = archetype.GetRaw<MeshFilter>();

        for (ComponentID i = 0; i < archetype.entityCount; i++)
        {
            auto& transform = transforms[i];
            auto& meshFilter = meshFilters[i];
            auto& meshRender = meshRenders[i];

            auto& mesh = meshFilter.mesh;
            for (size_t j=0;j<meshRender.numMaterials;j++)
            {
                auto& material = meshRender.materials[j];

                m_renderables.push_back({
                            material,
                            mesh,
                            static_cast<uint32_t>(j),
                            &transform.worldMatrix
                        });
            }
        }
    }
    */

    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        if (a.material != b.material)
            return a.material < b.material;

        if (a.mesh != b.mesh)
            return a.mesh < b.mesh;

        return a.submesh < b.submesh;
    });

    size_t i = 0;
    while (i < m_renderables.size())
    {
        const auto& first = m_renderables[i];

        std::vector<Matrix4x4> instanceMatrices;

        size_t j = i;

        while (j < m_renderables.size())
        {
            const auto& current = m_renderables[j];

            bool sameBatch =
                current.material == first.material &&
                current.mesh   == first.mesh &&
                current.submesh  == first.submesh;

            if (!sameBatch)
                break;

            instanceMatrices.push_back(*current.world);
            ++j;
        }

        RenderCommand cmd;
        cmd.material = AssetsManager::GetMaterial(first.material);
        cmd.mesh = AssetsManager::GetMesh(first.mesh);
        cmd.subMeshID = first.submesh;
        cmd.instances = instanceMatrices;

        Renderer::AddCommand(cmd);

        i = j;
    }

    m_renderables.clear();
}



