#pragma once

#include "default.hpp"

#include "physics/default.hpp"

#include "ecs/entity.hpp"

class BoxCollider : public Component {
    CLASS_FUNCTIONS
public:
    BoxCollider();

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    void build(const Vector3 &size);

    btCollisionShape *getShape();

    const Vector3 &getBoxSize() const;

    void setBoxSize(const Vector3 &value);
    
private:
    std::shared_ptr<btCollisionShape> m_CollisionShape;

    Vector3 boxSize;
};

REGISTER_COMPONENT(BoxCollider)

