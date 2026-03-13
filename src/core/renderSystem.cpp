#include "gr-base/core/renderSystem.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/gameObject.hpp"

#include "gr-base/ecs/behaviorStorage.hpp"

#include "gr-base/managers/batchingmanager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include <algorithm>
#include <iostream>

typedef struct Renderable
{
    MaterialID materialID;
    MeshID meshID;
} Renderable;

inline uint64_t CreateSortKey(const Renderable& r)
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

    for (auto &&[transformID, batchID] : m_instances)
    {
        Transform* transform = BehaviorStorage::GetComponent<Transform>(transformID);
        auto& modelMatrix = transform->getGlobalMatrix();

        auto &batch = BatchingManager::Get(batchID);
        batch.insert(modelMatrix);
    }
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

        std::vector<Renderable> objects;

        size_t count = meshIDs.size();
        for (size_t i=0;i<count;i++)
        {
            MaterialID materialID = materialIDs[i];
            MeshID meshID = meshIDs[i];

            objects.push_back({materialID, meshID});

            size_t key = 0;
            key ^= std::hash<size_t>{}(meshID) + 0x9e3779b9 + (key << 6) + (key >> 2);
            key ^= std::hash<size_t>{}(materialID) + 0x9e3779b9 + (key << 6) + (key >> 2);

            BatchID batchID = BatchingManager::Find(key);
            if (batchID == InvalidBatch)
                batchID = BatchingManager::Create(key, meshID, materialID);

            m_instances.emplace_back(transformID, batchID);
        }

        std::sort(objects.begin(), objects.end(), [](const Renderable& a, const Renderable& b) {
            return CreateSortKey(a) < CreateSortKey(b);
        });

        for (auto& r : objects)
        {
            std::cout << "MaterialID: " << r.materialID << " - " << r.meshID << std::endl;
        }
    }
}



