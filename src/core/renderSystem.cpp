#include "gr-base/core/renderSystem.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/gameObject.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/batchingmanager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <GL/glew.h>
#include <algorithm>
#include <iostream>

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
    m_markToRender.push_back(handle);
}

void RenderSystem::remove(GameObjectHandle handle)
{
    m_markToRender.erase(std::remove(m_markToRender.begin(), m_markToRender.end(), handle), m_markToRender.end());

    auto gameobject = GameObject::Get(handle);
    if (!gameobject)
        return;

    auto transformID = gameobject->GetID<Transform>();

    m_instances.erase(
        std::remove_if(m_instances.begin(), m_instances.end(),
            [transformID](const auto& instance) {
                return std::get<0>(instance) == transformID;
            }),
        m_instances.end()
    );
}

void RenderSystem::update()
{
    if (!m_markToRender.empty())
        setup(std::move(m_markToRender));

    for (auto&& [key, renderable] : m_renderable)
    {
        if (renderable.size() > 1)
        {
            std::vector<Matrix4x4> models;

            MaterialID m_materialID;
            MeshID m_meshID;

            for (auto&& [materialID, meshID, transformID] : renderable)
            {
                m_materialID = materialID;
                m_meshID = meshID;

                Transform* transform = BehaviorStorage::GetComponent<Transform>(transformID);

                auto &globalMatrix = transform->getGlobalMatrix();

                models.push_back(globalMatrix);
            }

            m_queueInstancingCmd[m_materialID][m_meshID] = models;
        } else
        {
            for (auto&& [materialID, meshID, transformID] : renderable)
                m_queueFirstCmd[materialID][meshID].push_back(transformID);
        }
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

        auto shader = material->GetShader(false);

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

            for (auto& transformID : transforms)
            {
                Transform* transform = BehaviorStorage::GetComponent<Transform>(transformID);

                auto &globalMatrix = transform->getGlobalMatrix();

                shader->SetUniform(u_model, &globalMatrix);

                glDrawElements(GL_TRIANGLES, mesh->getIndices().size(), GL_UNSIGNED_INT, nullptr);
            }

            drawCallCount += transforms.size();
        }
    }

    for (auto&& [materialID, sortByMesh] : m_queueInstancingCmd)
    {
        Material* material = AssetsManager::GetMaterial(materialID);

        auto shader = material->GetShader(true);

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

        for (auto&& [meshID, instances] : sortByMesh)
        {
            Mesh* mesh = AssetsManager::GetMesh(meshID);

            auto vertexarray = mesh->GetGpuMesh();
            if (lastVertexArray != vertexarray)
            {
                lastVertexArray = vertexarray;
                vertexarray->Bind();
            }

            auto pbo = mesh->GetIntancingBuffer();

            pbo->Bind();
            pbo->SetData(instances.data(), instances.size() * sizeof(Matrix4x4));
            pbo->Unbind();

            glDrawElementsInstanced(GL_TRIANGLES, mesh->getIndices().size(), GL_UNSIGNED_INT, nullptr, instances.size());

            drawCallCount++;
        }
    }

    // std::cout << "drawCallCount: " << drawCallCount << std::endl;

    m_queueInstancingCmd.clear();
    m_queueFirstCmd.clear();
}

void RenderSystem::setup(std::vector<GameObjectHandle> objects)
{
    for (auto& handle : objects)
    {
        auto gameobject = GameObject::Get(handle);

        auto meshFilter = gameobject->getComponent<MeshFilter>();
        auto meshRender = gameobject->getComponent<MeshRender>();

        auto transformID = gameobject->GetID<Transform>();

        if (meshFilter == nullptr || meshRender == nullptr)
        {
            Console::LogWarning("[RenderSystem] Objeto '%s' ignorado: MeshFilter ou MeshRender ausente.", gameobject->getName().c_str());
            continue;
        }

        auto materialIDs = meshRender->getMaterialIDs();
        auto meshIDs = meshFilter->getMesh();

        if (materialIDs.size() != meshIDs.size())
        {
            Console::LogError("[RenderSystem] Erro de configuracao no objeto '%s': Quantidade de materiais (%ld) nao condiz com a quantidade de sub-meshes (%ld).", gameobject->getName().c_str(), materialIDs.size(), meshIDs.size());
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

            uint64_t sortKey = CreateSortKey({materialID, meshID});

            m_renderable[sortKey].push_back({materialID, meshID, transformID});
        }
    }
}



