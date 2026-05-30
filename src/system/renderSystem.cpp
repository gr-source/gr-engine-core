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

            // const Matrix4x4& worldMatrix = transforms[i].worldMatrix;

            auto mesh = AssetsManager::GetMesh(meshFilter.meshID);
            if (mesh->GetGpuMesh() == nullptr)
                mesh->Build();

            auto& sphere = mesh->getBounds();

            Vector4 worldCenter =
                transform.worldMatrix * 
                    Vector4{sphere.center.x, sphere.center.y, sphere.center.z, 1.0f};

            float scale =
                std::max({
                    transform.scale.x,
                    transform.scale.y,
                    transform.scale.z
                });

            float radius =
                sphere.radius * scale;

            if (!Frustum::IsSphereInFrustum(
                    *reinterpret_cast<Vector3*>(worldCenter.data),
                    radius))
            {
                continue;
            }

            auto& materials = meshRenders[i].materials;
            if (materials.size() != mesh->GetSubMeshCount())
                continue;

            for (uint32_t j=0;j<materials.size();j++)
            {
                m_renderables.push_back({
                            meshFilters[i].meshID,
                            mesh,
                            j,
                            materials[j],
                            transform.worldMatrix
                        });
            }
        }
    }

    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        if (a.materialID != b.materialID)
            return a.materialID < b.materialID;

        if (a.meshID != b.meshID)
            return a.meshID < b.meshID;

        return a.subMeshID < b.subMeshID;
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
                current.materialID == first.materialID &&
                current.meshID   == first.meshID &&
                current.subMeshID  == first.subMeshID;

            if (!sameBatch)
                break;

            instanceMatrices.push_back(current.world);
            ++j;
        }

        i = j;
    }

    m_renderables.clear();
}



