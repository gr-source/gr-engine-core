#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/geometry/batchingRender.hpp"
#include "gr-base/core/status.hpp"

#include "gr-base/core/threadpool.hpp"
#include "gr-base/script/animator.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include "gr-base/core/geometry/mesh.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "gr-base/core/culling/frustum.hpp"

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::mBatchInstanceMap;

std::unordered_map<Batching *, std::string> Batching::mBatchRefToNameMap;

std::vector<Batching *> Batching::mBatchRefList;

void Batching::Initialize()
{
}

void Batching::Shutdown()
{
}

void Batching::RenderShadowPass() {
}

grm::size Batching::GetBatchCount() {
    return mBatchRefList.size();
}

Batching *Batching::FindBatchByName(const std::string &name) {
    auto it = mBatchInstanceMap.find(name);
    if (it == mBatchInstanceMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void Batching::RemoveBatch(Batching * batching)
{
    auto it = mBatchRefToNameMap.find(batching);
    if (it == mBatchRefToNameMap.end())
    {
        return;
    }

    mBatchInstanceMap.erase(it->second);

    mBatchRefList.erase(std::remove(mBatchRefList.begin(), mBatchRefList.end(), it->first), mBatchRefList.end());

    mBatchRefToNameMap.erase(it);
}

void Batching::RenderBatches()
{
    Status::BatchDrawCallRender = 0;
    Status::BatchRenderCount = 0;

    for (auto &batching : mBatchRefList)
    {
        if (!batching->isValid())
        {
            continue;
        }

        batching->draw();
    
        Status::BatchRenderCount++;
    }
}

Batching::Batching(std::vector<Mesh> *meshList) : m_meshList(meshList)
{
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
}

Batching::~Batching()
{
    Status::BatchDrawCallCount -= m_meshList->size();
}

struct VertexData {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;
    int entityID;
};

void Batching::buildMeshe()
{
    Status::BatchDrawCallCount += m_meshList->size();

    auto count = m_meshList->size();

    m_batchingRender.resize(count);

    for (std::size_t i=0;i<count;i++)
    {
        auto mesh = (*m_meshList)[i];

        m_batchingRender[i].build(&mesh);

        calcboundbox(&mesh);
    }
}

void Batching::insert(Transform *transform, std::vector<Material *> *materialList)
{
    if (m_instanceMap.find(transform) != m_instanceMap.end())
    {
        return;
    }

    auto &instance = m_instanceList.emplace_back();
    instance.materialList = materialList;
    instance.transform = transform;

    m_instanceMap.emplace(transform);
}

void Batching::remove(Transform *transform)
{
    auto it = m_instanceMap.find(transform);
    if (it == m_instanceMap.end())
    {
        return;
    }

    m_instanceList.erase(std::remove(m_instanceList.begin(), m_instanceList.end(), *it), m_instanceList.end());

    m_instanceMap.erase(it);
}

void Batching::draw() {
    for (auto &&[transform, materialList] : m_instanceList)
    {
        auto matrix = transform->getGlobalMatrix();

        if (!Frustum::IsCubeInFrustumOOBB(m_boundbox.min, m_boundbox.max, matrix))
        {
            continue;
        }
        
        int entityID = static_cast<int>(transform->entity->getEntityID());

        for (std::size_t i={};i<materialList->size();i++)
        {
            if (i >= m_batchingRender.size())
            {
                continue;
            }

            /*m_batchingRender[i]->updatePBO(sizeof(int), &entityID);*/
    
            auto material = (*materialList)[i];

            auto shader = material->getShader();
            if (!shader)
            {
                continue;
            }

            if (transform->getGameObject()->hasComponent<Animator>())
            {
                auto animator = transform->getGameObject()->getComponent<Animator>();


                shader->setUniform("u_boneMatrices", animator->m_FinalBoneMatrices.data());
            }

            shader->setUniform("u_model", &matrix);

            material->flush();

            shader->bind();
            shader->flush();

            m_batchingRender[i].render();

            shader->unbind();
        }
    }
}

void Batching::calcboundbox(const Mesh *mesh)
{
    for (auto vertex : mesh->getVertices())
    {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

bool Batching::isValid() const
{
    return !m_instanceList.empty();
}

bool Batching::isVisible() const {
    return false;
    /*return Frustum::IsCubeInFrustumOOBB(m_boundbox.min, m_boundbox.max, m_models[0]->getGlobalMatrix());*/
}












