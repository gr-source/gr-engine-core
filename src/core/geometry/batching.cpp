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

grr::gShader *Batching::s_shadowShader = nullptr;

void Batching::Initialize() {
    // AssetsManager::LoadShader(File::GetAsset("/shader/shadow/shadow.glsl"));
}

// TODO: falta descarregar o shader
void Batching::Shutdown() {
    if (s_shadowShader) {
        s_shadowShader = nullptr;
    }
}

void Batching::RenderShadowPass() {
    /*
    if (!s_shadowShader) {
        return;
    }

    auto lights = Light::GetListLight();
    if (lights.empty()) { return; }

    grr::gShader::SetUniform("u_light", lights[0]->getProjetion());

    for (auto &batch : s_batchedInstancesList) {
        if (!batch || !batch->isValid()) {
            continue;
        }
        
        batch->draw(s_shadowShader, false);
    }

    grr::gShader::Unbind();
    */
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

Batching::Batching(std::vector<Material *> *materials, BatchingFlags_ flags) : m_flags(flags), m_materials(materials), meshCount(0)
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

void Batching::buildMeshe(std::vector<Mesh> *meshList) {
    auto entity = m_models[0]->getGameObject()->getEntity();
    auto id = static_cast<int>(entity->getEntityID());

    meshCount = meshList->size();

    Status::BatchDrawCallCount += meshCount;

    for (auto &mesh : *meshList) {
        auto draw = &(*m_drawList.emplace_back(std::make_unique<BatchingDraw>()));
        
        draw->build(&mesh, id);
        
        calcboundbox(&mesh);
    }

    m_flags |= BatchingFlags_BuildMesh;
}

void Batching::insert(Transform *transform) {
    m_models.emplace_back(transform);
}

void Batching::remove(Transform *transform) {
    m_models.erase(std::remove(m_models.begin(), m_models.end(), transform), m_models.end());
}

void Batching::draw() {
    // auto model = m_models[0]->getGlobalMatrix();

    int idx[10] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::vector<InstancedData> models;

    for (auto transform : m_models) {
        auto entityID = transform->entity->getEntityID();

        models.emplace_back(InstancedData{static_cast<int>(entityID), transform->getGlobalMatrix()});
    }

    for (int i=0;i<m_materials->size();i++) {
        if (i >= m_drawList.size()) { continue; }

        int idxEn[10] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        
        auto draw = &(*m_drawList[i]);

        auto material = (*m_materials)[i];
        
        auto shader = material->getShader();
        if (!shader) { continue; }

        draw->updatePBO(models.data(), models.size());
        
        // shader->setUniform("u_model", 1, &model);
        // 0 - Albedo
        // 1 - Normal
        // 2 - Roughness
        // 3 - Metallic
        // 4 - Ao
        auto texture = material->getTexture(0);
        if (texture && texture->isValid()) {
            texture->bind(0);
            idxEn[0] = 1;
        }
        
        auto params = Vector3{material->getRoughness(), material->getMetallic(), material->getAmbientOcclusion()};
        auto albedo = material->getAlbedo();
        
        shader->setUniform("m_material.albedo", &albedo);
        shader->setUniform("m_material.params", &params);
        
        shader->setUniform("m_texture[0]", &idx[0]);
        shader->setUniform("m_activate[0]", &idxEn[0]);

        // material->flush();
        
        draw->m_vertexArray->bind();
        shader->bind();
        shader->flush();

        draw->draw();

        shader->unbind();
    }

    for (int i=0;i<10;i++) {
        grr::gTexture::Unbind(i);
    }
}

void Batching::calcboundbox(const Mesh *mesh) {
    for (auto vertex : mesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

bool Batching::isValid() const {
    return !m_models.empty() && (m_flags & BatchingFlags_BuildMesh);
}

bool Batching::isVisible() const {
    return Frustum::IsCubeInFrustumOOBB(m_boundbox.min, m_boundbox.max, m_models[0]->getGlobalMatrix());
}

BatchingDraw::BatchingDraw() : m_vertexArray(grr::gVertexArray::Create()), m_instanceCount(0),m_indexCount(0), m_VBO(grr::gVertexArray::CreateBuffer(BufferType::VBO)), m_EBO(grr::gVertexArray::CreateBuffer(BufferType::EBO)), m_PBO(grr::gVertexArray::CreateBuffer(BufferType::PBO))
{
}

BatchingDraw::~BatchingDraw()
{
    if (m_vertexArray) {
        grr::gVertexArray::Destroy(m_vertexArray);
    }

    if (m_VBO) {
        grr::gVertexArray::DeleteBuffer(m_VBO);
    }

    if (m_EBO) {
        grr::gVertexArray::DeleteBuffer(m_EBO);
    }

    if (m_PBO) {
        grr::gVertexArray::DeleteBuffer(m_PBO);
    }
}

void BatchingDraw::build(Mesh *mesh, int entityID) {
    auto vertices = mesh->getVertices();
    // for (auto vertice : mesh->getVertices()) {
    //     vertices.push_back(VertexData{vertice.position, vertice.normal, vertice.texCoord, vertice.mIdx, entityID});
    // }

    m_indexCount = vertices.size();
    
    m_vertexArray->bind();

    /*
    if (m_flags & BatchingFlags_HasIndices) {
        m_EBO = m_EBO != 0 ? m_EBO : grr::gVertexArray::CreateBuffer(BufferType::EBO);

        auto indices = mesh->getIndices();

        m_indexCount = indices.size();

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());
    }
    */
    
    
    grr::gVertexArray::Bind(m_VBO);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    // grr::gVertexArray::SetAttrib(3, 1,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));
    
    grr::gVertexArray::Bind(m_PBO);
    grr::gVertexArray::SetAttribI(3, 1, sizeof(InstancedData), reinterpret_cast<const void*>(offsetof(InstancedData, entityID)));
    grr::gVertexArray::SetAttribDivisor(3, 1);

    for (int i=0;i<4;i++) {
        grr::gVertexArray::SetAttrib(4 + i, 4, sizeof(InstancedData), reinterpret_cast<const void*>(offsetof(InstancedData, model) + i * sizeof(Vector4)));
        grr::gVertexArray::SetAttribDivisor(4 + i, 1);
    }

    m_vertexArray->unbind();
}

void BatchingDraw::draw()
{
    // if (m_flags & BatchingFlags_HasIndices) {
    //     grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_indexCount, nullptr);
    // } else {

    if (m_instanceCount > 0)
        grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, m_indexCount, m_instanceCount);
    else
        grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);

        Status::BatchDrawCallRender++;
    // }
}

void BatchingDraw::updatePBO(const InstancedData *data, std::size_t count)
{
    m_vertexArray->bind();
    grr::gVertexArray::Bind(m_PBO);
    grr::gVertexArray::UpdateResizeBuffer(count * sizeof(InstancedData), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, count * sizeof(InstancedData), data);

    m_instanceCount = count;
}
