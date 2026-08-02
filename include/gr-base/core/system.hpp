#pragma once

#include "gr-base/ecs/signature.hpp"
#include "gr-base/core/default.hpp"

class ISystem
{
protected:
    Scene* scene = nullptr;

public:
    virtual ~ISystem() {}

    virtual void Init() {}

    virtual void Update() = 0;

    virtual void OnGUI() {}

    virtual Signature GetWrite() const = 0;

    virtual Signature GetRead() const = 0;

    friend class Scene;
};

template <typename Read, typename Write>
class ISystemImpl : public ISystem
{
private:
    Signature GetRead() const override
    {
        return Read::Get();
    }

    Signature GetWrite() const override
    {
        return Write::Get();
    }
};



