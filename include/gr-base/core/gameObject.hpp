#pragma once

#include "default.hpp"

#include <gr-ecs/entity.hpp>

#include "gr-base/script/default.hpp"

class GameObject : public Entity
{
public:
    static GameObject* Instantiate(GameObject* other);

    static GameObject* Create(const std::string& name);

    static void DestroyImediate(GameObject* gameObject);

    static void Destroy(GameObject* gameObject);

    GameObject() = delete;

    GameObject(GameObjectID id);
    ~GameObject();

    std::string getName() const;

    void setName(const std::string& name);

    Transform* getTransform() const;

    GameObjectID getCacheID() const;

    void SetState(bool enable);

    void Copy(const GameObject* other);

private:
    Transform* m_transform;

    GameObjectID m_id;

    std::string m_name;
};


