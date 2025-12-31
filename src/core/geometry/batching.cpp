#include "gr-base/core/geometry/batching.hpp"

// core
#include "gr-base/managers/assetsManager.hpp"

 // core, geometry
#include "gr-base/core/geometry/material.hpp"

// ecs
#include "gr-base/core/console.hpp"
#include "gr-base/ecs/gameObject.hpp"

// component
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

// gr-render
#include <gr-render/gVertexArray.h>

// public
Batching::Batching(const std::vector<MeshID> &meshes) : m_meshes(meshes)
{}

Batching::~Batching()
{}

void Batching::insert(Transform *transform, const std::vector<MaterialID> &materials)
{
    if (materials.empty())
    {
        Console::LogError("[ Batching ] this model don't have material");
        return;
    }

    auto gameObject = transform->getGameObject();

    bool isanimated = gameObject->hasComponent<Animator>();
    bool isstatic = gameObject->getStatic();

    std::string key;
    for (auto &materialID : materials)
        key += AssetsManager::GetMaterialName(materialID);

    auto it = m_batchingdynamicmap.find(key);
    if (it == m_batchingdynamicmap.end())
    {
        m_batchingdynamicmap.emplace(key, std::make_shared<BatchingDynamic>(materials, m_meshes));

        m_batchingdynamiclist.push_back(m_batchingdynamicmap[key].get());
    }
                
    m_batchingdynamicmap[key]->insert(transform);
}

void Batching::render()
{
    for (auto &batching : m_batchingdynamiclist)
        batching->render();
}




