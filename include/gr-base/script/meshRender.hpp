#pragma once

#include "gr-base/core/geometry/batching.hpp"
#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/ecs/entity.hpp"

#include "default.hpp"

class MeshRender : public Component
{
    CLASS_FUNCTIONS
public:
    MeshRender(const MeshRender &other);
    MeshRender();

    void Init() override;

    void Destroy() override;

    std::vector<Material *> getMaterials() const;
    
    template <typename ...Args>
    Material *addMaterial(Args &&...args);
    
    Material *insertMaterial(Material *material);
    
    void removeMaterial(Material *material);

    bool hasMaterial(Material *material) const;

    Batching *getBatchGroup();

    void build();

    void Enable() override;

    void Disable() override;
    
    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

private:
    std::vector<Material *> m_materials;

    Batching *m_batched;

    Transform *m_transform;

    bool SetupShader(std::vector<Mesh> *meshList);
};

template <typename... Args>
inline Material *MeshRender::addMaterial(Args &&...args) {
    return insertMaterial(AssetsManager::CreateMaterial(std::forward<Args>(args)...));
}

REGISTER_COMPONENT(MeshRender, false)



