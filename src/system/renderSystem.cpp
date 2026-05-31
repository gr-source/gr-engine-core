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

static constexpr uint32_t BUCKET_COUNT = 1 << 16;

struct Bucket
{
    std::vector<const Matrix4x4*> instances;
};

size_t buckets[BUCKET_COUNT] = {(size_t)-1};

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
            auto& meshRender = meshRenders[i];

            auto& mesh = meshFilter.mesh;
            for (size_t j=0;j<meshRender.numMaterials;j++)
            {
                auto& material = meshRender.materials[j];

                uint64_t key =
                    ((uint64_t)material * 73856093u) ^
                    ((uint64_t)mesh      * 19349663u) ^
                    ((uint64_t)j   * 83492791u);

                uint32_t index = key & (BUCKET_COUNT - 1);
                
                if (buckets[index] == (size_t)-1)
                {

                    Renderable r;
                    r.material = material;
                    r.mesh = mesh;
                    r.submesh = j;

                    m_renderables.push_back(r);

                    buckets[index] = m_renderables.size() - 1;
                }

                m_renderables[buckets[index]].instances.push_back(&transform.worldMatrix);

                // std::cout << "Index: << " << index << std::endl;

                //.instances.push_back(&transform.worldMatrix);
            }
        }
    }

    for (auto& r : m_renderables)
    {
        std::vector<Matrix4x4> instances(r.instances.size());
        for (size_t i=0;i<r.instances.size();i++)
            instances[i] = *r.instances[i];

        r.instances.clear();

        RenderCommand cmd;
        cmd.material = AssetsManager::GetMaterial(r.material);
        cmd.mesh = AssetsManager::GetMesh(r.mesh);
        cmd.subMeshID = r.submesh;
        cmd.instances = instances.data();
        cmd.instanceCount = instances.size();

        Renderer::AddCommand(cmd);
    }

    /*
    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        if (a.material != b.material)
            return a.material < b.material;

        if (a.mesh != b.mesh)
            return a.mesh < b.mesh;

        return a.submeshID < b.submeshID;
    });
    */

    /*
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
                handle_get_first(current.material) == handle_get_first(first.material) &&
                handle_get_first(current.mesh)   == handle_get_first(first.mesh) &&
                current.submeshID  == first.submeshID;

            if (!sameBatch)
                break;

            instanceMatrices.push_back(current.world);
            ++j;
        }

        RenderCommand cmd;
        cmd.material = AssetsManager::GetMaterial(first.material);
        cmd.mesh = AssetsManager::GetMesh(first.mesh);
        cmd.subMeshID = first.submeshID;
        cmd.metrices = instanceMatrices;

        Renderer::AddCommand(cmd);

        i = j;
    }
    */

    // m_renderables.clear();
}



