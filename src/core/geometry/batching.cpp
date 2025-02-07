#include "gr-base/core/geometry/batching.hpp"

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

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::s_batchedInstances;

std::vector<Batching *> Batching::s_batchedInstancesList;

grr::gShader *Batching::s_shadowShader = nullptr;

void Batching::Initialize() {
    AssetsManager::LoadShader("shadow",
        File::GetAsset("/shader/shadow/shadow.frag"),
        File::GetAsset("/shader/shadow/shadow.vert"), &Batching::RegisterShaderShadow);
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
    return s_batchedInstancesList.size();
}

Batching *Batching::FindBatchByName(const std::string &name) {
    auto it = s_batchedInstances.find(name);
    if (it == s_batchedInstances.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void Batching::RemoveBatchByName(const std::string &name) {
    auto it = s_batchedInstances.find(name);
    if (it == s_batchedInstances.end()) {
        return;
    }

    s_batchedInstancesList.erase(std::remove(s_batchedInstancesList.begin(), s_batchedInstancesList.end(), &(*it->second)), s_batchedInstancesList.end());

    s_batchedInstances.erase(it);
}

void Batching::RenderBatches() {
    Status::BatchRenderCount = 0;

    for (auto &batching : s_batchedInstancesList) {
        if (!batching->isValid() || batching->isVisible()) {
            continue;
        }

        batching->draw();
    
        Status::BatchRenderCount++;
    }
}

Batching::Batching(const std::string &name, std::vector<Material *> *materials, grr::gShader *shader, BatchingFlags_ flags) : m_shader(shader), m_flags(flags), m_name(name), m_materials(materials), m_vertexArray(nullptr), m_indexCount(0), m_instanceCount(0), m_VBO(0), m_EBO(0), m_PBO(0) {}

Batching::~Batching() {
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

struct VertexData {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;
    int entityID;
};

void Batching::buildMeshe(Mesh *mesh) {
    auto entity = m_models[0]->getGameObject()->getEntity();

    m_vertexArray = m_vertexArray != nullptr ? m_vertexArray : grr::gVertexArray::Create();

    m_VBO = m_VBO != 0 ? m_VBO : grr::gVertexArray::CreateBuffer(BufferType::VBO);
    
    std::vector<VertexData> vertices;
    for (auto vertice : mesh->getVertices()) {
        vertices.push_back(VertexData{vertice.position, vertice.normal, vertice.texCoord, vertice.mIdx, static_cast<int>(entity->getEntityID())});
    }

    m_indexCount = vertices.size();
    
    m_vertexArray->bind();

    if (m_flags & BatchingFlags_HasIndices) {
        m_EBO = m_EBO != 0 ? m_EBO : grr::gVertexArray::CreateBuffer(BufferType::EBO);

        auto indices = mesh->getIndices();

        m_indexCount = indices.size();

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());
    }

    grr::gVertexArray::Bind(m_VBO);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(VertexData), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(VertexData), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3,  sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, position)));
    grr::gVertexArray::SetAttrib(1, 3,  sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, normal)));
    grr::gVertexArray::SetAttrib(2, 2,  sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1,  sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, mIdx)));
    grr::gVertexArray::SetAttribI(4, 1, sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, entityID)));

    m_vertexArray->unbind();

    calcboundbox(mesh);

    m_flags |= BatchingFlags_BuildMesh;
}

void Batching::insert(Transform *transform) {
    m_models.emplace_back(transform);
}

void Batching::remove(Transform *transform, bool bDelete) {
    m_models.erase(std::remove(m_models.begin(), m_models.end(), transform), m_models.end());

    if (m_models.empty() && bDelete) {
        Batching::RemoveBatchByName(m_name);
    }
}

void Batching::draw(grr::gShader *shader, bool bSetMaterial) {
    shader = shader != nullptr ? shader : m_shader;

    auto transform = m_models[0];
    auto go = transform->getGameObject();

    auto model = transform->getGlobalMatrix();

    shader->setUniform("u_model", 1, &model);

    /*
    auto animator =  go->getComponent<Animator>();
        auto &metrices = animator->getBoneData();
    if (animator) {
        
        grr::gShader::SetUniform<Matrix4x4>("u_boneMatrices", metrices.data());
    }
    */

    if (bSetMaterial)
        setupMaterial(shader);

    
    m_vertexArray->bind();

    shader->bind();
    shader->flush();

    if (m_flags & BatchingFlags_HasIndices) {
        grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_indexCount, nullptr);
    } else {
        grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);
    }

    for (int i=0;i<10;i++) {
        grr::gTexture::Unbind(i);
    }

    m_vertexArray->unbind();
    shader->unbind();
}

void Batching::setupMaterial(grr::gShader *shader) {
    int idx[10] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idxEn[10] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    char buff[256] = {0};

    for (int i=0;i<m_materials->size();i++) {
        auto &material = (*m_materials)[i];

        // 0 - Albedo
        // 1 - Normal
        // 2 - Roughness
        // 3 - Metallic
        // 4 - Ao
        auto texture = material->getTexture(0);
        if (texture && texture->isValid()) {
            texture->bind(i);
            idxEn[i] = 1;
        }

        auto params = Vector3{material->getRoughness(), material->getMetallic(), material->getAmbientOcclusion()};
        auto albedo = material->getAlbedo();

        sprintf(buff, "m_material[%d].albedo", i);
        shader->setUniform(buff, 1, &albedo);

        sprintf(buff, "m_material[%d].params", i);
        shader->setUniform(buff, 1, &params);
    }
    shader->setUniform("m_texture", 10, (const void *)idx);
    shader->setUniform("m_activate", 10, (const void *)idxEn);
}

void Batching::calcboundbox(const Mesh *mesh) {
    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    
    for (auto vertex : mesh->getVertices()) {
        m_boundbox.min = Math::min(m_boundbox.min, vertex.position);
        m_boundbox.max = Math::max(m_boundbox.max, vertex.position);
    }
}

void Batching::RegisterShaderShadow(grr::gShader *shader) {
    /*
    if (!shader) {
        return;
    }

    grr::gShader::Register("u_model", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    grr::gShader::Register("u_light", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    
    s_shadowShader = shader;
    */
}

bool Batching::isValid() const {
    return !m_models.empty() && (m_flags & BatchingFlags_BuildMesh);
}

#include "gr-base/core/culling/frustum.hpp"
bool Batching::isVisible() const {
    return !Frustum::IsCubeInFrustumOOBB(m_boundbox.min, m_boundbox.max, m_models[0]->getGlobalMatrix());
}
