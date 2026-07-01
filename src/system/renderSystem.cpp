#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/world.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/light.hpp"

#include "gr-base/spatial/bvhtree.hpp"

void RenderSystem::Update(Scene* scene)
{
    Entity cameraEntity = scene->GetCamera();
    if (cameraEntity == INVALID_ENTITY)
        return;

    auto world = scene->GetWorldDynamic();
    auto bvh = scene->GetBVHtree();

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

            Renderer::Submit(
                        material,
                        mesh,
                        static_cast<uint32_t>(j),
                        &transform->worldMatrix
                    );
        }
    }
}




