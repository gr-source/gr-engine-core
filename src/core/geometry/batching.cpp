#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/status.hpp"

#include "gr-base/core/threadpool.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <filesystem>
#include <fstream>
#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include "gr-base/core/geometry/mesh.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <tuple>

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::mBatchInstanceMap;

std::unordered_map<Batching *, std::string> Batching::mBatchRefToNameMap;

std::vector<Batching *> Batching::mBatchRefList;

void Batching::Initialize()
{
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
        if (!batching->isValid())
        {
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
        {
            auto draw = &(*m_batchingRenderList.emplace_back(std::make_unique<BatchingRender>(BatchingRenderFlags_Default)));
            
            draw->build(&mesh);
        }

        {
            auto draw = &(*m_batchingRenderInstancedList.emplace_back(std::make_unique<BatchingRender>(BatchingRenderFlags_Instanced)));
            
            draw->build(&mesh);
        }
        
        calcboundbox(&mesh);
    }

    m_flags |= BatchingFlags_BuildMesh;
}

void Batching::insert(Transform *transform, std::vector<Material *> *materialList) {
    std::string name;
    for (auto material : *materialList)
    {
        name += AssetsManager::GetRefMaterialToName(material);
    }

    auto it = m_optmizeMap.find(name);
    if (it == m_optmizeMap.end())
    {
        m_optmizeCountMap[name]++;

        for (auto material : *materialList)
        {
            // if (material->getShader() != mShaderOptimize[0])
            //    material->setShader(mShaderOptimize[0]);
            
            Threadpool::AddTask(&Batching::SetUpdateShaderOptmize, material, material->getShader(), 0);
        }
        
        m_batchedModelNoOptimizeList.emplace_back(materialList, transform);

        m_optmizeMap.emplace(name, std::make_tuple(transform, OptimizeFlags_NoOptimize));
    } else
    {
        // move to instanced
        {
            auto &&[last_transform, flags] = it->second;

            if (flags == OptimizeFlags_NoOptimize)
            {
                // remove da lista de objstos n~ao otimizados.
                m_batchedModelNoOptimizeList.erase(std::remove(m_batchedModelNoOptimizeList.begin(), m_batchedModelNoOptimizeList.end(), last_transform), m_batchedModelNoOptimizeList.end());

                auto ptr_to_tuple = &m_modelsInstacedFromRender.emplace_back(std::make_tuple(materialList, std::vector<Transform *>()));

                m_batchedModelInstancedMap.emplace(name, ptr_to_tuple);

                for (auto material : *materialList)
                {
                    Threadpool::AddTask(&Batching::SetUpdateShaderOptmize, material, material->getShader(), 1);
                    // if (material->getShader() != mShaderOptimize[1])
                    //    material->setShader(mShaderOptimize[1]);
                }
                
                // move o last_transform para a lista de mesh instance
                auto &&[m, vector] = (*m_batchedModelInstancedMap[name]);
                vector.push_back(last_transform);
                
                flags = OptimizeFlags_InstanceMesh;
            }

            m_optmizeCountMap[name]++;

            auto &&[m, vector] = (*m_batchedModelInstancedMap[name]);

            
            vector.push_back(transform);
        }
    }
    
    m_materialRef[transform] = name;
}

void Batching::remove(Transform *transform) {
    auto it = m_materialRef.find(transform);
    if (it != m_materialRef.end())
    {
        auto &&[last_transform, flags] = m_optmizeMap[it->second];
        
        switch (flags)
        {
            case OptimizeFlags_NoOptimize:
                m_batchedModelNoOptimizeList.erase(std::remove(m_batchedModelNoOptimizeList.begin(), m_batchedModelNoOptimizeList.end(), transform), m_batchedModelNoOptimizeList.end());
                break;
            case OptimizeFlags_InstanceMesh: {
                auto &&[m, vector] = (*m_batchedModelInstancedMap[it->second]);

                vector.erase(std::remove(vector.begin(), vector.end(), transform), vector.end());
                break;
            }
        default:
            break;
        }

        m_optmizeCountMap[it->second]--;

        if (flags == OptimizeFlags_InstanceMesh)
        {
            auto &&[materialList, vector] = (*m_batchedModelInstancedMap[it->second]);

            if (vector.size() <= 1)
            {
                for (auto material : *materialList)
                {
                    /*
                    if (material->getShader() != mShaderOptimize[0])
                    {
                        material->setShader(mShaderOptimize[0]);
                    }
                    */


                    Threadpool::AddTask(&Batching::SetUpdateShaderOptmize, material, material->getShader(), 0);
                }

                if (transform == last_transform)
                {
                    last_transform = vector.back();
                }

                m_modelsInstacedFromRender.erase(std::remove(m_modelsInstacedFromRender.begin(), m_modelsInstacedFromRender.end(), (*m_batchedModelInstancedMap[it->second])), m_modelsInstacedFromRender.end());

                m_batchedModelInstancedMap.erase(it->second);

                m_batchedModelNoOptimizeList.emplace_back(materialList, last_transform);
            
                flags = OptimizeFlags_NoOptimize;
            }
        }

        if (m_optmizeCountMap[it->second] <= 0)
        {
            m_optmizeCountMap.erase(it->second);

            m_optmizeMap.erase(it->second);

            m_materialRef.erase(it);
        }
    }
}

void Batching::draw() {
    for (auto &&[materialList, transform] : m_batchedModelNoOptimizeList)
    {
        auto matrix = transform->getGlobalMatrix();
        
        int entityID = static_cast<int>(transform->entity->getEntityID());

        for (std::size_t i={};i<materialList->size();i++)
        {
            if (i >= m_batchingRenderList.size())
            {
                continue;
            }

            m_batchingRenderList[i]->updatePBO(sizeof(int), &entityID);
    
            auto material = (*materialList)[i];
            
            auto shader = material->getShader();
            if (!shader)
            {
                continue;
            }
    
            shader->setUniform("u_model", &matrix);
    
            material->flush();
            
            shader->bind();
            shader->flush();
    
            m_batchingRenderList[i]->render();

            shader->unbind();
        }
    }
        
    for (auto &&[materialList, vector] : m_modelsInstacedFromRender)
    {

        std::vector<InstancedData> m_instanceList;

        for (auto transform : vector)
        {
            m_instanceList.push_back({static_cast<int>(transform->entity->getEntityID()), transform->getGlobalMatrix()});
        }

        for (int i=0;i<materialList->size();i++) {
            if (i >= m_batchingRenderInstancedList.size()) { continue; }

            
            auto draw = &(*m_batchingRenderInstancedList[i]);
            
            draw->setInstancedCount(m_instanceList.size());
            
            auto material = (*materialList)[i];
            
            auto shader = material->getShader();
            if (!shader) { continue; }
    
            draw->updatePBO(m_instanceList.size() * sizeof(InstancedData), m_instanceList.data());
    
            material->flush();
            
            shader->bind();
            shader->flush();
    
            draw->render();
    
            shader->unbind();
        }
    }
}

void Batching::calcboundbox(const Mesh *mesh) {
    for (auto vertex : mesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

bool Batching::isValid() const
{
    return (!m_batchedModelInstancedMap.empty() || !m_batchedModelNoOptimizeList.empty()) && (m_flags & BatchingFlags_BuildMesh);
}

bool Batching::isVisible() const {
    return false;
    // return Frustum::IsCubeInFrustumOOBB(m_boundbox.min, m_boundbox.max, m_models[0]->getGlobalMatrix());
}

void Batching::SetUpdateShaderOptmize(Material *material, grr::gShader *shader, int id)
{
    std::string path;
    try {
        path = AssetsManager::GetShaderRefToName(shader);
    }
    catch (const std::exception&) {
        return;
    }

    std::ifstream is(path);
    if (!is.is_open())
    {
        return;
    }

    std::hash<std::string> hash;

    auto temp_file = std::filesystem::temp_directory_path() / (std::to_string(hash(path)) + ".glsl");

    std::ofstream os(temp_file);

    std::string line;
    while (std::getline(is, line))
    {
        if (line.find("define_type") != std::string::npos)
        {
            os << line << std::endl;
            switch (id) {
                case 0:
                    os << "#define USE_MODEL" << std::endl;
                    break;
                case 1:
                    os << "#define USE_MODEL" << std::endl;
                    os << "#define USE_INSTANCED" << std::endl;
                    break;
                default:
                    break;
            }
            continue;
        }
        os << line << std::endl;
    }

    os.close();

    is.close();

    AssetsManager::UpdateShader(temp_file, shader, &Material::setShader, material);
}













