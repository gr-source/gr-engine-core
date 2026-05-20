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

    Signature requiredMask = Type::GetMask<Transform, MeshRender, MeshFilter>();

    static std::vector<MaterialID> m_batchlookup;

    auto array = scene->GetECS()->GetRawArcheType();

    for (size_t i=0;i<scene->GetECS()->GetArcheTypeCount();i++)
    {
        ArchetypeStorage& archetype = array[i];

        if ((archetype.GetMask() & requiredMask) == requiredMask)
        {
            auto transforms = archetype.GetRaw<Transform>();
            auto meshRenders = archetype.GetRaw<MeshRender>();
            auto meshFilters = archetype.GetRaw<MeshFilter>();

            if (archetype.Count() >= m_batchlookup.size())
                m_batchlookup.resize(archetype.Count() + 1, InvalidNode);
            
            for (ComponentID i=0;i<archetype.Count();i++)
            {
                EntityID entityID = handle_get_first(archetype.GetEntity(i));

                Matrix4x4 model = transforms[i].worldMatrix;

                if (m_batchlookup[entityID] == InvalidNode)
                {
                    auto& materials = meshRenders[i].materials;
                    MeshID meshID = meshFilters[i].meshID;

                    auto mesh = AssetsManager::GetMesh(meshID);
                    if (mesh->GetGpuMesh() == nullptr)
                        mesh->Build();

                    int subMeshCount = mesh->GetSubMeshCount();
                    if (materials.size() == subMeshCount)
                    {
                        RenderCommand cmd;
                        for (int i=0;i<subMeshCount;i++)
                        {

                            auto materialID = materials[i];
                            auto& subMesh = mesh->GetSubMesh(i);

                            cmd.materialID = materialID;
                            cmd.meshID = meshID;
                            cmd.subMeshID = i;
                            cmd.matrix = model;

                            MaterialID batchID = InvalidNode;

                            Renderer::AddCommand(cmd, batchID);

                            m_batchlookup[entityID] = batchID;
                        }
                    }

                } else
                {
                    RenderCommand cmd;
                    cmd.matrix = model;

                    Renderer::AddCommand(cmd, m_batchlookup[entityID]);
                }
            }
        }
    }

    // static std::vector<MaterialID> m_batchlookup;

    // for (auto&& [e, transform, meshRender, meshFilter] : world->Query<Transform, MeshRender, MeshFilter>())
    {
        /*
        EntityID entityID = handle_get_first(e);
        if (entityID >= m_batchlookup.size())
            m_batchlookup.resize(entityID + 1, InvalidNode);

        Matrix4x4 model = transform->worldMatrix;

        if (m_batchlookup[entityID] == InvalidNode)
        {
            auto& materials = meshRender->materials;
            MeshID meshID = meshFilter->meshID;

            auto mesh = AssetsManager::GetMesh(meshID);
            if (mesh->GetGpuMesh() == nullptr)
                mesh->Build();

            int subMeshCount = mesh->GetSubMeshCount();
            if (materials.size() == subMeshCount)
            {
                RenderCommand cmd;
                for (int i=0;i<subMeshCount;i++)
                {
                    auto materialID = materials[i];
                    auto& subMesh = mesh->GetSubMesh(i);

                    cmd.materialID = materialID;
                    cmd.meshID = meshID;
                    cmd.subMeshID = i;
                    cmd.matrix = model;

                    MaterialID batchID = InvalidNode;

                    Renderer::AddCommand(cmd, batchID);

                    m_batchlookup[entityID] = batchID;
                }
            }
        } else
        {
            RenderCommand cmd;
            cmd.matrix = model;

            Renderer::AddCommand(cmd, m_batchlookup[entityID]);
        }
        */

        // Matrix4x4 mvp =
        //     cameraData->viewProjection *
        //     model;
    }

    // Graphics::Present(
    //     camera->renderTarget
    // );
}



