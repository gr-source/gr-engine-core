#pragma once

#include "default.hpp"

#include "gr-base/script/collider.hpp"

#include <gr-math/gmath.hpp>

class BoxCollider : public Collider
{
public:
    BoxCollider();

    void Init() override;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;

    void build(const Vector3 &size);

    const Vector3 &getBoxSize() const;

    void setBoxSize(const Vector3 &value);
    
private:
    Vector3 boxSize;
};

REGISTER_COMPONENT(BoxCollider, CLASS_TO_HASH(Collider))


