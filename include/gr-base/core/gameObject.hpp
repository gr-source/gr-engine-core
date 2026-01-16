#pragma once

#include "default.hpp"

#include "gr-base/ecs/entity.hpp"

#include "gr-base/script/default.hpp"

class GameObject : public Entity
{
public:
    static GameObjectID Instantiate(GameObject* other);

    static GameObjectID Create(const std::string& name);

    static GameObject* Get(GameObjectID handle);

    static void DestroyImediate(GameObject* gameObject);

    static void DestroyImediate(GameObjectID gameObjectID);

    static void Destroy(GameObject* gameObject);

    static void Destroy(GameObjectID gameObjectID);

    GameObject() = delete;

    GameObject(GameObjectID id);
    ~GameObject();

    std::string getName() const;

    void setName(const std::string& name);

    Transform* getTransform() const;

    void SetState(bool enable);

    void Copy(GameObjectID other);

private:
    std::string m_name;
};


