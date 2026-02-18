#pragma once

#include "default.hpp"

#include "gr-base/ecs/entity.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/file/default.hpp"

class GameObject : public Entity
{
public:
    static GameObjectHandle Instantiate(GameObject* other);

    static GameObjectHandle Create(const std::string& name);

    static GameObject* Get(GameObjectHandle handle);

    static void DestroyImediate(GameObject* gameObject);

    static void DestroyImediate(GameObjectHandle gameObjectID);

    static void Destroy(GameObject* gameObject);

    static void Destroy(GameObjectHandle gameObjectID);

    GameObject() = delete;

    GameObject(GameObjectHandle id);
    ~GameObject();

    std::string getName() const;

    void setName(const std::string& name);

    Transform* GetTransform() const;

    void SetState(bool enable);

    void Copy(GameObjectHandle other);

    void Serialize(Stream* stream) const;

    void Deserialize(Stream* stream);

    void SetScene(Scene* scene);

    Scene* GetScene() const;

    void SetIsStatic(bool b);

    bool GetIsStatic() const;

    GameObjectHandle FindChildByName(const std::string& name);

private:
    std::string m_name;

    Scene* m_scene;

    bool m_isStatic;

    static void Serialize(GameObjectHandle id, Stream* stream);

    static void Deserialize(Stream* stream, GameObjectHandle id, ComponentID parent);
};


