#include "gr-base/core/renderSystem.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

#include <GL/glew.h>

RenderSystem* RenderSystem::s_RenderSystem = nullptr;

void RenderSystem::SetInstance(RenderSystem *renderSystem)
{
    s_RenderSystem = renderSystem;
}

void RenderSystem::Insert(Entity handle)
{
    s_RenderSystem->insert(handle);
}

void RenderSystem::Remove(Entity handle)
{
    s_RenderSystem->remove(handle);
}

void RenderSystem::Update()
{
    s_RenderSystem->update();
}

void RenderSystem::Begin(const Camera* camera)
{
    s_RenderSystem->begin(camera);
}

void RenderSystem::End()
{
    s_RenderSystem->end();
}

void RenderSystem::Renderer()
{
    s_RenderSystem->renderer();
}

// private
void RenderSystem::insert(Entity handle)
{
    // m_markToRender.emplace(handle);
}

void RenderSystem::remove(Entity gameObjectID)
{
    // m_markToRender.erase(gameObjectID);
}

void RenderSystem::update()
{
    /*
    if (!m_markToRender.empty())
    {
        for (auto& handle : m_markToRender)
            setup(handle);
        m_markToRender.clear();
    }

    /*
    for (auto&& [handle, tuple] : m_renderable)
    {
        auto&& [submeshID, transforms] = tuple;

        MaterialID materialID = handle_get_first(handle);
        MeshID meshID = handle_get_second(handle); 

        size_t count = transforms.size();

        auto& vec = m_queueFirstCmd[materialID][handle_create(meshID, submeshID)];
        vec.resize(count);

        for (size_t i=0;i<count;i++)
        {
            Transform* transform = ComponentStorage::GetComponent<Transform>(transforms[i]);

            vec[i] = transform->getGlobalMatrix();
        }
    }
    */
}

// sort by
// [shaderID][materialID][meshID]
void RenderSystem::begin(const Camera* camera)
{
    /*
    auto scene = Scene::GetCurrent();

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    for (auto prefab : AssetsManager::GetAllShader3D())
    {
        for (auto shader : prefab->GetShaders())
        {
            shader->bind();
            shader->setUniform("u_mat_projection", &projection);
            shader->setUniform("u_mat_view",       &view);
            shader->setUniform("u_mat_eye",        &position);

            setuplight(scene, shader);
            shader->unbind();
        }
    }

    Frustum::CalculateFrustum(projection * view);
    */
}

void RenderSystem::end()
{}

void RenderSystem::renderer()
{
    /*
    for (auto& batch : m_batches)
    {
        bool bInstancing = batch.instances.size() > 1;

        auto materialInstance = batch.materialInstance;

        auto material = materialInstance->GetMaterial();

        auto shader = material->GetShaderPrefab()->Get(bInstancing ? ShaderPrefabFlags_Instancing : ShaderPrefabFlags_none);
        materialInstance->SetShader(shader);

        materialInstance->sync();

        auto mesh = batch.mesh;

        auto vertexarray = mesh->GetGpuMesh();

        bool enable[10] = {false};
        int textureID[10] = {0};

        shader->bind();
        shader->setUniform("u_mat_texture", textureID);
        shader->setUniform("u_mat_enable", enable);

        materialInstance->flush();

        vertexarray->Bind();

        auto &submesh = mesh->GetSubMesh(batch.submeshID);

        if (bInstancing)
        {
            batch.matrices.resize(batch.instances.size());

            for (size_t i=0;i<batch.instances.size();i++)
                batch.matrices[i] = ComponentStorage::GetComponent<Transform>(batch.instances[i])->getGlobalMatrix();

            auto pbo = mesh->GetIntancingBuffer();

            pbo->Bind();
            pbo->SetData(batch.matrices.data(), batch.matrices.size() * sizeof(Matrix4x4));
            pbo->Unbind();

            glDrawElementsInstanced(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)), batch.instances.size());
        } else
        {
            shader->set_uniform("u_mat_model", ComponentStorage::GetComponent<Transform>(batch.instances[0])->getGlobalMatrix());

            glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)));
        }

    }
    */
}

uint64_t make_handle(MaterialID materialID, MeshID meshID, uint32_t submeshIndex)
{
    return ((uint64_t)materialID << 32) |
           ((uint64_t)meshID << 16) |
           (uint64_t)submeshIndex;
}

void RenderSystem::setup(Entity gameObjectID)
{
    /*
    auto gameobject = GameObject::Get(gameObjectID);

    auto meshFilter = gameobject->getComponent<MeshFilter>();
    auto meshRender = gameobject->getComponent<MeshRender>();

    auto transformID = gameobject->GetID<Transform>();

    if (meshFilter == nullptr || meshRender == nullptr)
        return Console::LogWarning("[RenderSystem] Objeto '%s' ignorado: MeshFilter ou MeshRender ausente.", gameobject->GetName().c_str());

    auto materialIDs = meshRender->getMaterialIDs();
    auto meshID = meshFilter->getMeshID();

    if (meshID == ID_INVALID)
        return;

    auto mesh = AssetsManager::GetMesh(meshID);

    if (materialIDs.size() != mesh->GetSubMeshCount())
        return Console::LogError("[RenderSystem] Erro de configuracao no objeto '%s': Quantidade de materiais (%ld) nao condiz com a quantidade de sub-meshes (%ld).", gameobject->GetName().c_str(), materialIDs.size(), mesh->GetSubMeshCount());

    auto vertexarray = mesh->GetGpuMesh();

    if (vertexarray == nullptr)
        mesh->Build();

    std::hash<MaterialID> hash;

    size_t submeshcount = mesh->GetSubMeshCount();
    for (size_t i=0;i<submeshcount;i++)
    {
        MaterialID materialID = materialIDs[i];

        for (size_t j=0;j<mesh->GetSubMeshCount();j++)
        {
            uint64_t key = make_handle(materialID, meshID, j);

            auto it = m_batchIndex.find(key);
            if (it == m_batchIndex.end())
            {
                auto& batch = m_batches.emplace_back();
                batch.materialInstance = std::make_shared<MaterialInstance>(AssetsManager::GetMaterial(materialID));
                batch.mesh = mesh;
                batch.submeshID = j;

                m_batchIndex[key] = m_batches.size() - 1;
            }

            m_batches[m_batchIndex[key]].instances.push_back(transformID);
        }
    }
    */
}

void RenderSystem::setuplight(const Scene* scene, gr::Shader *shader)
{
    /*
    auto lights = scene->GetLights();

    int numLight = static_cast<int>(lights.size());

    shader->set_uniform("u_mat_numLights", numLight);

    if (lights.empty())
        return;

    for (int i=0;i<numLight;i++)
    {
        auto light = ComponentStorage::GetComponent<Light>(lights[i]);
        if (light == nullptr)
            break;

        auto position   = light->getPosition();
        auto forward    = light->getForward();
        auto color      = light->getColor();
        auto type       = light->getType();

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
    */
}



