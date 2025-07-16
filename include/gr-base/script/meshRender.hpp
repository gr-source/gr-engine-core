#pragma once

#include "gr-base/script/baserender.hpp"

class MeshRender : public BaseRender
{
    CLASS_FUNCTIONS
public:
    MeshRender();

    void onInit() override;

    void onRelease() override;

    bool onSerialize(Stream *stream) override;

    bool onDeserialize(Stream *stream) override;

    void build() override;

private:
};

REGISTER_COMPONENT(MeshRender)



