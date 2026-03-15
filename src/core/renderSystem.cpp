#include "gr-base/core/renderSystem.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/gameObject.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/batchingmanager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <GL/glew.h>
#include <algorithm>

inline uint64_t CreateSortKey(const RenderCommand& r)
{
    uint64_t key = 0;
    key |= (uint64_t)r.materialID << 32;
    key |= (uint64_t)r.meshID;
    return key;
}

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

        uint64_t key = it->second;

        auto& vec = m_renderable[key];

        vec.erase(
            std::remove(vec.begin(), vec.end(), myTransformID),
            vec.end()
        );

        if (m_renderable[key].empty())
            m_renderable.erase(key);

        m_objectsToRenderKey.erase(it);
    }
}

void RenderSystem::update()
{
    if (!m_markToRender.empty())
    {
        setup(m_markToRender);
        m_markToRender.clear();
    }

    for (auto&& [handle, transforms] : m_renderable)
    {
        MaterialID materialID = handle_get_id(handle);
        MeshID meshID = handle_get_version(handle); 

        size_t count = transforms.size();

        std::vector<Matrix4x4> models(count);
        for (size_t i=0;i<count;i++)
        {
            Transform* transform = BehaviorStorage::GetComponent<Transform>(transforms[i]);

            auto &globalMatrix = transform->getGlobalMatrix();

            models[i] = globalMatrix;
        }

        m_queueFirstCmd[materialID][meshID] = models;
    }
}

void RenderSystem::renderer()
{
    const gr::vertex_array* lastVertexArray = nullptr;
    grr::gShader* lastShader = nullptr;
    Material* lastMaterial = nullptr;

    size_t drawCallCount = 0;

    for (auto&& [materialID, sortByMesh] : m_queueFirstCmd)
    {
        Material* material = AssetsManager::GetMaterial(materialID);

        auto shader = material->GetShader(material->bEnableInstancing);

        if (shader != lastShader)
        {
            lastShader = shader;

            bool enable[10] = {false};
            int textureID[10] = {0};

            shader->bind();
            shader->setUniform("u_texture", textureID);
            shader->setUniform("u_enable", enable);
        }

        if (lastMaterial != material)
        {
            lastMaterial = material;

            material->flush(shader);
        }

        ShaderID u_model = shader->findUniform("u_model");

        for (auto&& [meshID, transforms] : sortByMesh)
        {
            Mesh* mesh = AssetsManager::GetMesh(meshID);

            auto vertexarray = mesh->GetGpuMesh();
            if (lastVertexArray != vertexarray)
            {
                lastVertexArray = vertexarray;
                vertexarray->Bind();
            }

            if (material->bEnableInstancing)
            {
                auto pbo = mesh->GetIntancingBuffer();

                pbo->Bind();
                pbo->SetData(transforms.data(), transforms.size() * sizeof(Matrix4x4));
                pbo->Unbind();

                glDrawElementsInstanced(GL_TRIANGLES, mesh->getIndices().size(), GL_UNSIGNED_INT, nullptr, transforms.size());

                drawCallCount++;
            } else
            {
                for (auto& matrix : transforms)
                {
                    shader->SetUniform(u_model, &matrix);

                    glDrawElements(GL_TRIANGLES, mesh->getIndices().size(), GL_UNSIGNED_INT, nullptr);
                }

                drawCallCount += transforms.size();
            }
        }
    }

    std::cout << "drawCallCount: " << drawCallCount << std::endl;

    m_queueFirstCmd.clear();
}

void RenderSystem::setup(std::unordered_set<GameObjectHandle> objects)
{
    for (auto& gameObjectID : objects)
    {
        auto gameobject = GameObject::Get(gameObjectID);

        auto meshFilter = gameobject->getComponent<MeshFilter>();
        auto meshRender = gameobject->getComponent<MeshRender>();

        auto transformID = gameobject->GetID<Transform>();

        if (meshFilter == nullptr || meshRender == nullptr)
        {
            Console::LogWarning("[RenderSystem] Objeto '%s' ignorado: MeshFilter ou MeshRender ausente.", gameobject->GetName().c_str());
            continue;
        }

        auto materialIDs = meshRender->getMaterialIDs();
        auto meshIDs = meshFilter->getMesh();

        if (materialIDs.size() != meshIDs.size())
        {
            Console::LogError("[RenderSystem] Erro de configuracao no objeto '%s': Quantidade de materiais (%ld) nao condiz com a quantidade de sub-meshes (%ld).", gameobject->GetName().c_str(), materialIDs.size(), meshIDs.size());
            continue;
        }

        size_t count = meshIDs.size();
        for (size_t i=0;i<count;i++)
        {
            MaterialID materialID = materialIDs[i];
            MeshID meshID = meshIDs[i];

            Mesh* mesh = AssetsManager::GetMesh(meshID);

            auto vertexarray = mesh->GetGpuMesh();

            if (vertexarray == nullptr)
                mesh->Build();

            uint64_t handle = handle_create(materialID, meshID);

            m_renderable[handle].push_back(transformID);

            m_objectsToRenderKey.emplace(gameObjectID, handle);
        }
    }
}



