#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/behavior.hpp"

class MeshFilter : public BehaviorImpl<MeshFilter>
{
public:
    MeshFilter();

    inline void SetMesh(MeshID id)
    {
        m_meshID = id;
    }

    inline MeshID getMeshID() const
    {
        return m_meshID;
    }

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;
    
private:
    MeshID m_meshID;
};

REGISTER_COMPONENT(MeshFilter, (uint64_t)-1)

