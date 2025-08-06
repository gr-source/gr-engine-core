#include "gr-base/core/geometry/batching.hpp"

// core
#include "gr-base/core/assetsManager.hpp"

 // core, geometry
#include "gr-base/core/geometry/batchingstatic.hpp"
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
Batching::Batching(std::vector<Mesh> *meshlist) : m_count(0), m_meshlist(meshlist)
{}

Batching::~Batching()
{}

void Batching::insert(Transform *transform, const std::vector<Material *> &materiallist)
{
    if (materiallist.empty())
    {
        return Console::LogError("[ Batching ] this model don't have material");
    }

    auto gameObject = transform->getGameObject();

    bool isanimated = gameObject->hasComponent<Animator>();
    bool isstatic = gameObject->getStatic();

    std::string key;
    for (auto &material : materiallist)
        key += AssetsManager::GetRefMaterialToName(material);

    auto it = m_batchingdynamicmap.find(key);
    if (it == m_batchingdynamicmap.end())
    {
        m_batchingdynamicmap.emplace(key, std::make_unique<BatchingDynamic>(materiallist, m_meshlist));

        m_batchingdynamiclist.push_back(m_batchingdynamicmap[key].get());
    }
                
    m_batchingdynamicmap[key]->insert(transform);

    m_count++;
}

void Batching::erase(Transform *transform)
{
    for (auto &batching : m_batchingdynamiclist)
    {
        batching->erase(transform);
    }
}

void Batching::render()
{
    for (auto &batching : m_batchingdynamiclist)
        batching->render();
}



