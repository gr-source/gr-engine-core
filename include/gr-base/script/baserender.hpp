#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/behavior.hpp"

class BaseRender : public BehaviorImpl<BaseRender>
{
public:
    BaseRender();

    virtual bool Serialize(Stream *stream) const override;

    virtual bool Deserialize(Stream *stream) override;

    void addMaterial(MaterialID id);

    bool hasMaterial(MaterialID id) const;

    std::vector<MaterialID> getMaterialIDs() const;

    void removeMaterial(MaterialID id);

private:
    std::vector<MaterialID> m_materials;

    friend class SkinnedMeshRender;
    friend class MeshRender;
};

REGISTER_COMPONENT(BaseRender)




