#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/status.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"
#include "gr-base/script/light.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include "gr-base/core/geometry/mesh.hpp"

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::mBatchInstanceMap;

std::unordered_map<Batching *, std::string> Batching::mBatchRefToNameMap;

std::vector<Batching *> Batching::mBatchRefList;

void Batching::Initialize() {
    // AssetsManager::LoadShader(File::GetAsset("/shader/shadow/shadow.glsl"));
}

// TODO: falta descarregar o shader
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

void Batching::RenderBatches() {
    Status::BatchDrawCallRender = 0;
    Status::BatchRenderCount = 0;

    for (auto &batching : mBatchRefList) {
        if (!batching->isValid()) {
            continue;
        }

        batching->draw();
    
        Status::BatchRenderCount++;
    }
}

Batching::Batching(std::vector<Mesh> *meshList) : m_meshList(meshList), meshCount(0), optimizeFlags(OptimizeFlags_NoOptimize)
{
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
}

Batching::~Batching()
{
    Status::BatchDrawCallCount -= meshCount;
}

struct VertexData {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;
    int entityID;
};

void Batching::buildMeshe() {
    meshCount = m_meshList->size();

    Status::BatchDrawCallCount += meshCount;

    for (auto &mesh : *m_meshList) {
        auto draw = &(*m_batchingRenderList.emplace_back(std::make_unique<BatchingRender>(BatchingRenderFlags_Default)));
        
        draw->build(&mesh);
        
        calcboundbox(&mesh);
    }

    m_flags |= BatchingFlags_BuildMesh;
}

void Batching::insert(Transform *transform, std::vector<Material *> *materialList) {
    m_batchedModelList.emplace_back(materialList, transform);
}

void Batching::remove(Transform *transform) {
    m_batchedModelList.erase(std::remove(m_batchedModelList.begin(), m_batchedModelList.end(), transform), m_batchedModelList.end());
}

void Batching::draw() {
    for (auto &&[materialList, transform] : m_batchedModelList)
    {
        auto matrix = transform->getGlobalMatrix();
        
        for (std::size_t i={};i<materialList->size();i++) {
            if (i >= m_batchingRenderList.size()) { continue; }
    
            auto material = (*materialList)[i];
            
            auto shader = material->getShader();
            if (!shader) { continue; }
    
            shader->setUniform("u_model", &matrix);
    
            material->flush();
            
            shader->bind();
            shader->flush();
    
            m_batchingRenderList[i]->render();

            shader->unbind();
        }
    }
    /*
    const InstancedData *instancedData = nullptr;
    std::size_t size = 0;

    // if (optimizeFlags == OptimizeFlags_InstanceMesh)
    // {
            auto entityID = transform->entity->getEntityID();
    
            m_instanceList.emplace_back(InstancedData{static_cast<int>(entityID), transform->getGlobalMatrix()});
        }
        instancedData = m_instanceList.data();
        size = m_instanceList.size();
    // }

    for (int i=0;i<m_materials->size();i++) {
        if (i >= m_drawList.size()) { continue; }

        auto draw = &(*m_drawList[i]);

        auto material = (*m_materials)[i];
        
        auto shader = material->getShader();
        if (!shader) { continue; }

        if (instancedData != nullptr)
        {
            switch (optimizeFlags)
            {
            case OptimizeFlags_InstanceMesh:
                draw->updatePBO(instancedData, size);
                break;
            case OptimizeFlags_NoOptimize:
                shader->setUniform("u_model", &instancedData->model);
                break;
            default:
                break;
            }
        }

        material->flush();
        
        draw->m_vertexArray->bind();
        shader->bind();
        shader->flush();

        draw->draw();

        shader->unbind();
    }

    if (optimizeFlags == OptimizeFlags_InstanceMesh)
    {
        m_instanceList.clear();
    }
    */
}

void Batching::calcboundbox(const Mesh *mesh) {
    for (auto vertex : mesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

bool Batching::isValid() const {
    return !m_batchedModelList.empty() && (m_flags & BatchingFlags_BuildMesh);
}

bool Batching::isVisible() const {
    return false;
    // return Frustum::IsCubeInFrustumOOBB(m_boundbox.min, m_boundbox.max, m_models[0]->getGlobalMatrix());
}
