#pragma once

#include "gr-base/render/pipeline/cameraRenderData.hpp"

class IPipeline
{
public:
    virtual ~IPipeline() = default;

    virtual void Initialize() {}

    virtual void Render(CameraRenderData& camera) = 0;
};


