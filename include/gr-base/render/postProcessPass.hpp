#pragma once

class IPostProcessPass
{
public:
    virtual ~IPostProcessPass() = default;

    virtual void Initialize() {}

    virtual void Shutdown() {}

    virtual void Execute() = 0;
};
