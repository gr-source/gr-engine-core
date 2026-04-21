#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/component.hpp"

class BaseRender : public ComponentImpl<BaseRender>
{
public:
    BaseRender();

    virtual bool Serialize(Stream *stream) const override;

    virtual bool Deserialize(Stream *stream) override;

    inline void SetMaterial(const std::vector<MaterialID>& materials)
    {
        m_materials = materials;
    }

    inline std::vector<MaterialID> getMaterialIDs() const
    {
        return m_materials;
    }

private:
    std::vector<MaterialID> m_materials;
};

REGISTER_COMPONENT(BaseRender, (uint64_t)-1)




