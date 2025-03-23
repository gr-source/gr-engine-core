#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/status.hpp"

#include "gr-base/core/threadpool.hpp"
#include "gr-base/script/animator.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <filesystem>
#include <fstream>
#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include "gr-base/core/geometry/mesh.hpp"

#include <algorithm>
#include <memory>
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

void Batching::buildMeshe() {
    Status::BatchDrawCallCount += m_meshList->size();

    for (auto &mesh : *m_meshList) {
        auto draw = &(*m_batchingRender.emplace_back(std::make_unique<BatchingRender>(BatchingRenderFlags_Default)));
            
        draw->build(&mesh);

        calcboundbox(&mesh);
    }
}

void Batching::insert(Transform *transform, std::vector<Material *> *materialList)
{
    if (m_instanceMap.find(transform) != m_instanceMap.end())
    {
        return;
    }

    m_instanceMap.emplace(transform, std::make_unique<Instance>(materialList, transform));

    auto instance = &(*m_instanceMap[transform]);

    m_instanceList.push_back(instance);
}

void Batching::remove(Transform *transform)
{
    auto it = m_instanceMap.find(transform);
    if (it == m_instanceMap.end())
    {
        return;
    }

    m_instanceList.erase(std::remove(m_instanceList.begin(), m_instanceList.end(), &(*it->second)), m_instanceList.end());

    m_instanceMap.erase(it);
}

void Batching::draw() {
    for (auto instance : m_instanceList)
    {
        auto &&[transform, materialList] = *instance;
        
        auto matrix = transform->getGlobalMatrix();
        
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
    
            m_batchingRender[i]->render();

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
    return !m_instanceList.empty();
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

    std::cout << path << ": Convert to " << (id == 0 ? "One Draw" : "Instanced") << std::endl;

    os.close();

    is.close();

    AssetsManager::UpdateShader(temp_file, shader, &Material::setShader, material);
}













