#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/view.hpp"
#include "gr-base/ecs/ecs.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/managers/default.hpp"
#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

void RenderSystem::Update(Scene* scene)
{
    auto world = scene->GetECS();

    Entity cameraEntity = scene->GetCamera();

    Transform* cameraTransform =
        world->GetComponent<Transform>(cameraEntity);

    Camera* camera =
        world->GetComponent<Camera>(cameraEntity);

    CameraData* cameraData =
        world->GetComponent<CameraData>(cameraEntity);

    Renderer::SetRenderTarget(
        camera->rendertarget
    );

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

    auto array = scene->GetECS()->GetRawArcheType();

    const SubMesh* cacheSubMesh = nullptr;

    Mesh* cacheMesh = nullptr;
    int submeshIndex = (int)-1;

    MeshID cacheMeshID = InvalidNode;

    RenderCommand cmd;

    for (size_t a = 0; a < scene->GetECS()->GetArcheTypeCount(); a++)
    {
        ArchetypeStorage& archetype = array[a];

        if ((archetype.GetMask() & requiredMask) != requiredMask)
            continue;

        auto transforms  = archetype.GetRaw<Transform>();
        auto meshRenders = archetype.GetRaw<MeshRender>();
        auto meshFilters = archetype.GetRaw<MeshFilter>();

        ComponentID count = archetype.Count();

        for (ComponentID i = 0; i < count; i++)
        {
            const Matrix4x4& model = transforms[i].worldMatrix;

            MeshID meshID = meshFilters[i].meshID;

            if (cacheMeshID != meshID)
            {
                cacheMesh = AssetsManager::GetMesh(meshID);

                if (cacheMesh->GetGpuMesh() == nullptr)
                    cacheMesh->Build();

                cacheMeshID = meshID;
            }

            auto& materials = meshRenders[i].materials;

            int subMeshCount = cacheMesh->GetSubMeshCount();

            if (materials.size() != subMeshCount)
                continue;

            for (int j = 0; j < subMeshCount; j++)
            {
                if (submeshIndex != j)
                {
                    cacheSubMesh = &cacheMesh->GetSubMesh(j);
                    submeshIndex = j;
                }
                cmd.materialID = materials[j];
                cmd.meshID = meshID;
                cmd.subMeshID = j;
                cmd.matrix = model;
                cmd.mesh = cacheMesh;
                cmd.cacheSubMesh = cacheSubMesh;

                Renderer::AddCommand(cmd);
            }
        }
    }

}



