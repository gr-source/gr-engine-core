#pragma once

#include "gr-base/core/geometry/batchingdynamic.hpp"
#include "gr-base/script/baserender.hpp"
#include <vector>

class MeshRender : public BaseRender
{
    CLASS_FUNCTIONS
public:
    MeshRender();

    void onInit() override;

    void onRelease() override;

    void build();

    bool onSerialize(Stream *stream) override;

    bool onDeserialize(Stream *stream) override;

    void build(std::vector<Mesh> *meshlist) override;

    BatchingDynamic *batchingdynamic;

private:
};

REGISTER_COMPONENT(MeshRender)



