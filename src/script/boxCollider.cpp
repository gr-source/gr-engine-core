#include "gr-base/script/boxCollider.hpp"

#include "gr-base/physics/physicsScene.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/gameObject.hpp"

#include "gr-base/script/transform.hpp"

BoxCollider::BoxCollider() : boxSize(Vector3::one)
{}

void BoxCollider::Init()
{
    auto transform = GetGameObject()->GetTransform();

    build(transform->getLocalScale());
}

bool BoxCollider::Serialize(Stream *stream) const
{
    stream->write(&boxSize, sizeof(Vector3));
    return true;
}

bool BoxCollider::Deserialize(Stream *stream)
{
    stream->read(reinterpret_cast<char *>(&boxSize), sizeof(Vector3));
    return true;
}


void BoxCollider::build(const Vector3 &size)
{
    shape = std::make_shared<btBoxShape>(btVector3(1.0f, 1.0f, 1.0f));
        
    shape->setLocalScaling(btVector3(size.x, size.y, size.z));
}

const Vector3 &BoxCollider::getBoxSize() const {
    return boxSize;
}

void BoxCollider::setBoxSize(const Vector3 &value) {
    boxSize = value;

    build(value);
}
