#include "gr-base/script/boxCollider.hpp"

#include "gr-base/physics/physicsScene.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/transform.hpp"

BoxCollider::BoxCollider() : m_CollisionShape(nullptr), boxSize(Vector3::one)
{}

void BoxCollider::Init()
{
    auto transform = GetComponent<Transform>();

    build(transform->getLocalScale());
}

bool BoxCollider::Serialize(Stream *stream)
{
    stream->write(&boxSize, sizeof(Vector3));
    return true;
}

bool BoxCollider::Deserialize(Stream *stream) {
    stream->read(reinterpret_cast<char *>(&boxSize), sizeof(Vector3));
    return true;
}


void BoxCollider::build(const Vector3 &size) {
    if (!m_CollisionShape)
        m_CollisionShape = std::make_shared<btBoxShape>(btVector3(1.0f, 1.0f, 1.0f));
        
    m_CollisionShape->setLocalScaling(btVector3(size.x, size.y, size.z));
}

btCollisionShape *BoxCollider::getShape() {
    return &(*m_CollisionShape);
}

const Vector3 &BoxCollider::getBoxSize() const {
    return boxSize;
}

void BoxCollider::setBoxSize(const Vector3 &value) {
    boxSize = value;

    build(value);
}
