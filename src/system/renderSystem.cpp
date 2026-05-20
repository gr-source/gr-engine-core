#include "gr-base/system/renderSystem.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/view.hpp"
#include "gr-base/ecs/ecs.hpp"

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

    for (auto&& [e, transform, meshRender, meshFilter] : world->Query<Transform, MeshRender, MeshFilter>())
    {
        Matrix4x4 model =
            transform->worldMatrix;

        Matrix4x4 mvp =
            cameraData->viewProjection *
            model;

        auto& materials = meshRender->materials;
        MeshID meshID = meshFilter->meshID;

        auto mesh = AssetsManager::GetMesh(meshID);
        if (mesh->GetGpuMesh() == nullptr)
            mesh->Build();

        int subMeshCount = mesh->GetSubMeshCount();
        if (materials.size() == subMeshCount)
        {
            for (int i=0;i<subMeshCount;i++)
            {
                auto materialID = materials[i];
                auto& subMesh = mesh->GetSubMesh(i);

                RenderCommand cmd;
                cmd.materialID = materialID;
                cmd.meshID = meshID;
                cmd.subMeshID = i;
                cmd.matrix = model;

                Renderer::AddCommand(cmd);
            }
        }
    }

    // Graphics::Present(
    //     camera->renderTarget
    // );
}



