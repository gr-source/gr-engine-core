#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

MeshRender::MeshRender() : m_batched(nullptr), m_transform(nullptr) {}

MeshRender::MeshRender(const MeshRender &other) : m_transform(nullptr), m_batched(nullptr), m_materials(other.m_materials) {}

void MeshRender::Init() {
    m_transform = GetComponent<Transform>();
}

void MeshRender::Destroy() {
    if (!m_batched) {
        return;
    }
    m_batched->remove(GetComponent<Transform>());
    m_batched = nullptr;
}

std::vector<Material *> MeshRender::getMaterials() const {
    return m_materials;
}

void MeshRender::removeMaterial(Material *material) {
    m_materials.erase(std::remove(m_materials.begin(), m_materials.end(), material), m_materials.end());
}

Batching *MeshRender::getBatchGroup() {
    return m_batched;
}

// TODO: falta orimizar e corrigir erros do material;
void MeshRender::build() {
    if (!m_transform) {
        m_transform = entity->getComponent<Transform>();
    }

    if (!entity->hasComponent<MeshFilter>()) {
        return Console::LogError("[ MeshRender ] MeshFilter component fo not found.");
    }

    auto meshFilter = entity->getComponent<MeshFilter>();
    
    auto mesh = meshFilter->getMesh();
    if (!mesh) {
        return Console::LogError("[ MeshRender ] Object do not has mesh.");
    }

    if (m_batched) {
        m_batched->remove(m_transform);
        m_batched = nullptr;
    }

    auto shader = AssetsManager::FindShader(File::GetAsset("/shader/model/default_model.glsl"));
    if (!shader) {
        return AssetsManager::LoadShader(
            File::GetAsset("/shader/model/default_model.glsl"),
            &MeshRender::SetupShader, this, mesh
        );
    }

    SetupShader(mesh, shader);
}

void MeshRender::Enable() {
    if (!m_batched) {
        return;
    }
    m_batched->insert(GetComponent<Transform>());
}

void MeshRender::Disable() {
    if (!m_batched) {
        return;
    }
    m_batched->remove(GetComponent<Transform>());
}

bool MeshRender::Serialize(Stream *stream) {
    bool hasMaterial = !m_materials.empty();

    stream->write(reinterpret_cast<const char *>(&hasMaterial), sizeof(bool));

    if (!hasMaterial) {
        return true;
    }

    auto size = m_materials.size();
    stream->write(reinterpret_cast<const char *>(&size), sizeof(std::size_t));

    for (auto material : m_materials) {
        auto name = AssetsManager::GetRefMaterialToName(material);
        auto sname = name.size();

        stream->write(&sname, sizeof(size_t));

        stream->write(name.c_str(), sname * sizeof(char));
    }
    return true;
}

bool MeshRender::Deserialize(Stream *stream) {
    bool hasMaterial;

    stream->read(reinterpret_cast<char *>(&hasMaterial), sizeof(bool));

    if (!hasMaterial) {
        return true;
    }
    
    std::size_t size = 0;
    std::size_t sname = 0;
    stream->read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
    if (size <= 0) {
        return false;
    }

    m_materials.clear();

    for (std::size_t i={};i<size;i++) {
        stream->read(reinterpret_cast<char *>(&sname), sizeof(std::size_t));

        std::string name(sname, '\0');
        stream->read(name.data(), sname * sizeof(char));

        auto material = AssetsManager::FindMaterial(name);
        if (material) {
            m_materials.push_back(material);
        }
    }

    return true;
}

bool MeshRender::SetupShader(Mesh *mesh, grr::gShader *shader) {
    m_batched = Batching::FindBatchByName(getGameObject()->getName());
    if (!m_batched) {
        m_batched = Batching::AddBatch(getGameObject()->getName(), &m_materials, shader, mesh->hasIndices() ? BatchingFlags_HasIndices : 0);
        m_batched->insert(m_transform);
        m_batched->buildMeshe(mesh);
    }
    return true;
}




