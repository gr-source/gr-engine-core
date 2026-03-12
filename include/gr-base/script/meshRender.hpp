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

    void Destroy() override;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;
};

REGISTER_COMPONENT(MeshRender, (uint64_t)-1)



