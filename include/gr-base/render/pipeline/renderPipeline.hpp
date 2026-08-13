#pragma once

class IRenderPipeline
{
public:
    virtual ~IRenderPipeline() = default;

    virtual void Initialize() {}

    virtual void Execute() = 0;
};


