#include "core/geometry/batching.hpp"

// #include "renderManager.h"

#include "core/assetsManager.hpp"
// #include "core/statistics.hpp"
#include "core/event.hpp"

#include "script/transform.hpp"
#include "script/animator.hpp"
#include "script/light.hpp"

// #include "culling/frustum.h"

// #include "scriptManager/scriptManager.hpp"

#include "ecs/gameObject.hpp"

#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include <core/geometry/mesh.hpp>

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
    // Statistics::m_drawRenderCount = 0;
    // Statistics::m_drawCallCount = 0;
    // Statistics::m_totalRender = 0;

    for (auto &batching : s_batchedInstancesList) {
        if (!batching->isValid()) {
            continue;
        }
        
        batching->draw();
    
        // Statistics::m_drawCallCount++;
    }
}

Batching::Batching(const std::string &name, std::vector<std::shared_ptr<Material>> *materials, grr::gShader *shader, BatchingFlags_ flags) : m_shader(shader), m_flags(flags), m_name(name), m_materials(materials), m_vertexArray(nullptr), m_indexCount(0), m_instanceCount(0), m_VBO(0), m_EBO(0), m_PBO(0) {}

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

/*
struct VertexData {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;
    int m_index[4];
    float m_weight[4];
};

void Batching::buildMeshe(Mesh *mesh) {
    m_vertexArray = m_vertexArray != nullptr ? m_vertexArray : grr::gVertexArray::Create();

    m_VBO = m_VBO != 0 ? m_VBO : grr::gVertexArray::CreateBuffer(BufferType::VBO);
    
    auto vertices = mesh->getVertices();
    auto bones = mesh->getBoneWeight();

    m_indexCount = vertices.size();
    
    m_vertexArray->bind();

    std::vector<VertexData> buffer;
    for (int i=0;i<m_indexCount;i++) {
        auto vertex = vertices[i];
        auto bone = bones[i];

        auto &data = buffer.emplace_back(VertexData{vertex.position, vertex.normal, vertex.texCoord, vertex.mIdx});
        for (int i=0;i<4;i++) {
            data.m_index[i] = bone.m_index[i];
            data.m_weight[i] = bone.m_weight[i];
        }
    }

    if (m_flags & BatchingFlags_HasIndices) {
        m_EBO = m_EBO != 0 ? m_EBO : grr::gVertexArray::CreateBuffer(BufferType::EBO);

        auto indices = mesh->getIndices();

        m_indexCount = indices.size();

        std::cout << "Indices: " << m_indexCount << std::endl;

        grr::gVertexArray::Bind(m_EBO);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::u32), indices.data());
    }

    grr::gVertexArray::Bind(m_VBO);
    grr::gVertexArray::UpdateResizeBuffer(buffer.size() * sizeof(VertexData), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, buffer.size() * sizeof(VertexData), buffer.data());
    // grr::gVertexArray::SetBufferUpdate(vertices.size() * sizeof(Vertex3D), bones.size() * sizeof(BoneWeight), bones.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, mIdx)));
    
    grr::gVertexArray::SetAttribI(4, 4, sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, m_index)));
    grr::gVertexArray::SetAttrib(5, 4,  sizeof(VertexData), reinterpret_cast<const void*>(offsetof(VertexData, m_weight)));

    m_vertexArray->unbind();

    m_flags |= BatchingFlags_BuildMesh;
}
*/

void Batching::buildMeshe(Mesh *mesh) {
    m_vertexArray = m_vertexArray != nullptr ? m_vertexArray : grr::gVertexArray::Create();

    m_VBO = m_VBO != 0 ? m_VBO : grr::gVertexArray::CreateBuffer(BufferType::VBO);
    
    auto vertices = mesh->getVertices();

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
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
    grr::gVertexArray::SetAttrib(3, 1, sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, mIdx)));

    m_vertexArray->unbind();

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

    grr::gShader::SetUniform("u_model", transform->getGlobalMatrix());

    /*
    auto animator =  go->getComponent<Animator>();
        auto &metrices = animator->getBoneData();
    if (animator) {
        
        grr::gShader::SetUniform<Matrix4x4>("u_boneMatrices", metrices.data());
    }
    */
    
    if (bSetMaterial)
        setupMaterial();

    grr::gShader::Bind(shader);
    
    m_vertexArray->bind();

    grr::gShader::Flush();

    if (m_flags & BatchingFlags_HasIndices) {
        grr::gVertexArray::DrawElements(PrimitiveType::TRIANGLES, m_indexCount, nullptr);
    } else {
        grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, m_indexCount);
    }

    for (int i=0;i<10;i++) {
        grr::gTexture::Bind(nullptr, i);
    }
}

void Batching::setupMaterial() {
    int idx[10] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int idxEn[10] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (auto i=0;i<m_materials->size();i++) {
        auto &material = (*m_materials)[i];

        // 0 - Albedo
        // 1 - Normal
        // 2 - Roughness
        // 3 - Metallic
        // 4 - Ao
        auto texture = material->getTexture(0);
        if (texture && texture->isValid()) {
            grr::gTexture::Bind(texture, i);
            idxEn[i] = 1;
        }

        grr::gShader::SetUniform("m_material[" + std::to_string(i) + "].albedo", material->getAlbedo());
        grr::gShader::SetUniform("m_material[" + std::to_string(i) + "].params", Vector3({material->getRoughness(), material->getMetallic(), material->getAmbientOcclusion()}));
    }
    grr::gShader::SetUniform("m_texture", (const int *)idx);
    grr::gShader::SetUniform("m_activate", (const int *)idxEn);
}

void Batching::RegisterShaderShadow(grr::gShader *shader) {
    if (!shader) {
        return;
    }

    grr::gShader::Register("u_model", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    grr::gShader::Register("u_light", UniformType::MAT4, 1, Matrix4x4::identityMatrix);
    
    s_shadowShader = shader;
}

bool Batching::isValid() const {
    return !m_models.empty() && (m_flags & BatchingFlags_BuildMesh);
}







