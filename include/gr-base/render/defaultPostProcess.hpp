#pragma once

#include "gr-base/render/postProcessPass.hpp"

class DefaultPostProcess : public IPostProcessPass
{
public:
private:
    void Initialize() override;

    void Execute() override;
};


