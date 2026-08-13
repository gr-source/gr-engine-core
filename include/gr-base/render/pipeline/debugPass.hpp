#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

class DebugPass : public IRenderPass
{
public:
private:
    void Execute() override;
};

