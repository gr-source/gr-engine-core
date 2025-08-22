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

    bool onSerialize(Stream *stream) override;

    bool onDeserialize(Stream *stream) override;

    void build();

    void build(std::vector<Mesh> *meshlist) override;

    BatchingDynamic *batchingdynamic;

private:

    BatchID m_batchID;
};

REGISTER_COMPONENT(MeshRender)



