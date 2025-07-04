#pragma once

#include "gr-base/script/baserender.hpp"

class MeshRender : public BaseRender
{
    CLASS_FUNCTIONS
public:
    MeshRender();

    void Init() override;

    void Destroy() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    void build() override;

private:
};

REGISTER_COMPONENT(MeshRender)



