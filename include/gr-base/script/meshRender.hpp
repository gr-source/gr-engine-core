#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/script/baserender.hpp"
#include <vector>

class MeshRender : public BaseRender
{
public:
    MeshRender();
    ~MeshRender() override;

    void Init() override;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;

    void build(const std::vector<MeshID> &meshes);
};

REGISTER_COMPONENT(MeshRender)



