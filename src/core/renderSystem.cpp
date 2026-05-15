#include "gr-base/core/renderSystem.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/scene.hpp"
#include "gr-base/ecs/componentstorage.hpp"

#include "gr-base/externals/handle.h"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"

#include <GL/glew.h>
#include <algorithm>

RenderSystem* RenderSystem::s_RenderSystem = nullptr;

void RenderSystem::SetInstance(RenderSystem *renderSystem)
{
    s_RenderSystem = renderSystem;
}

void RenderSystem::Insert(GameObjectHandle handle)
{
    s_RenderSystem->insert(handle);
}

void RenderSystem::Remove(GameObjectHandle handle)
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
void RenderSystem::insert(GameObjectHandle handle)
{
    m_markToRender.emplace(handle);
}

void RenderSystem::remove(GameObjectHandle gameObjectID)
{
    m_markToRender.erase(gameObjectID);

    auto it = m_objectsToRenderKey.find(gameObjectID);
    if (it != m_objectsToRenderKey.end())
    {
        auto myTransformID = GameObject::Get(gameObjectID)->GetID<Transform>();

        for (auto& key : it->second)
        {
            auto&& [submeshID, vec] = m_renderable[key];

            vec.erase(
                std::remove(vec.begin(), vec.end(), myTransformID),
                vec.end()
            );

            if (vec.empty())
                m_renderable.erase(key);
        }

        m_objectsToRenderKey.erase(it);
    }
}

void RenderSystem::update()
{
    if (!m_markToRender.empty())
    {
        for (auto& handle : m_markToRender)
            setup(handle);
        m_markToRender.clear();
    }

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
}

// sort by
// [shaderID][materialID][meshID]
void RenderSystem::begin(const Camera* camera)
{
    auto scene = Scene::GetCurrent();

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    auto list = AssetsManager::GetShaderList(ShaderType::SHADER3D);
    for (auto &shader : list)
    {
        if (!shader)
            continue;

        shader->bind();
        shader->setUniform("u_mat_projection", &projection);
        shader->setUniform("u_mat_view",       &view);
        shader->setUniform("u_mat_eye",        &position);

        setuplight(scene, shader);
        shader->unbind();
    }

    Frustum::CalculateFrustum(projection * view);
}

void RenderSystem::end()
{}

void RenderSystem::renderer()
{
    const gr::vertex_array* lastVertexArray = nullptr;
    gr::Shader* lastShader = nullptr;
    Material* lastMaterial = nullptr;

    size_t drawCallCount = 0;

    for (auto&& [materialID, sortByMesh] : m_queueFirstCmd)
    {
        Material* material = AssetsManager::GetMaterial(materialID);

        auto shader = material->GetShader();

        if (shader != lastShader)
        {
            lastShader = shader;

            bool enable[10] = {false};
            int textureID[10] = {0};

            shader->bind();
            shader->setUniform("u_mat_texture", textureID);
            shader->setUniform("u_mat_enable", enable);
        }

        if (lastMaterial != material)
        {
            lastMaterial = material;

            material->flush(shader);
        }

        ShaderID u_model = shader->findUniform("u_mat_model");

        for (auto&& [handle, transforms] : sortByMesh)
        {
            uint32_t meshID = handle_get_first(handle);

            Mesh* mesh = AssetsManager::GetMesh(meshID);

            auto vertexarray = mesh->GetGpuMesh();
            if (lastVertexArray != vertexarray)
            {
                lastVertexArray = vertexarray;
                vertexarray->Bind();
            }

            if (material->supportsInstancing())
            {
                auto pbo = mesh->GetIntancingBuffer();

                pbo->Bind();
                pbo->SetData(transforms.data(), transforms.size() * sizeof(Matrix4x4));
                pbo->Unbind();

                // glDrawElementsInstanced(GL_TRIANGLES, mesh->getIndices().size(), GL_UNSIGNED_INT, nullptr, transforms.size());

                drawCallCount++;
            } else
            {
                auto &submesh = mesh->GetSubMesh(handle_get_second(handle));

                for (auto& matrix : transforms)
                {
                    shader->SetUniform(u_model, &matrix);

                    // for (int i=0;i<submeshcount;i++)
                    {
                        glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)));
                    }
                }

                drawCallCount += transforms.size();
            }
        }
    }

    m_queueFirstCmd.clear();
}

void RenderSystem::setup(GameObjectHandle gameObjectID)
{
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

    std::vector<uint64_t> handles;

    size_t submeshcount = mesh->GetSubMeshCount();
    for (size_t i=0;i<submeshcount;i++)
    {
        MaterialID materialID = materialIDs[i];

        Mesh* mesh = AssetsManager::GetMesh(meshID);

        auto vertexarray = mesh->GetGpuMesh();

        if (vertexarray == nullptr)
            mesh->Build();

        uint64_t handle = handle_create(materialID, meshID);

        auto&& [submeshID, vec] = m_renderable[handle];
        submeshID = i;

        vec.push_back(transformID);

        handles.push_back(handle);
    }

    m_objectsToRenderKey.emplace(gameObjectID, handles);
}

void RenderSystem::setuplight(const Scene* scene, gr::Shader *shader)
{
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
}



