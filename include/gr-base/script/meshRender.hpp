#pragma once


#include "core/geometry/batching.hpp"
#include "core/geometry/material.hpp"

#include "default.hpp"

#include "ecs/entity.hpp"

class MeshRender : public Component {
    CLASS_FUNCTIONS
public:
    MeshRender();

    void Init() override;

    void Destroy() override;

    std::vector<std::shared_ptr<Material>> &getMaterials();

    template <typename ...Args>
    Material *addMaterial(Args &&...args);

    Batching *getBatchGroup();

    void build();

    void Enable() override;

    void Disable() override;
    
    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

private:
    std::vector<std::shared_ptr<Material>> m_materials;

    Batching *m_batched;

    Transform *m_transform;

    bool SetupShader(Mesh *mesh, grr::gShader *shader);
};

template <typename... Args>
inline Material *MeshRender::addMaterial(Args &&...args) {
    return &(*m_materials.emplace_back(std::make_shared<Material>(std::forward<Args>(args)...)));
}

REGISTER_COMPONENT(MeshRender);

