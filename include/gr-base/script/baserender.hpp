#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/entity.hpp"

class BaseRender : public Component
{
    CLASS_FUNCTIONS
public:
    BaseRender();

    void onInit() override;

    void onRelease() override;

    void onEnable() override;

    void onDisable() override;

    bool onSerialize(Stream *stream) override;

    bool onDeserialize(Stream *stream) override;

    virtual void build(std::vector<Mesh> *meshlist) {}

    void addMaterial(MaterialID id);

    bool hasMaterial(MaterialID id) const;

    const std::vector<MaterialID> &getMaterial() const;

private:
    std::vector<MaterialID> m_materials;

    // BatchID m_batchID;

    friend class SkinnedMeshRender;
    friend class MeshRender;
};

REGISTER_COMPONENT(BaseRender)




