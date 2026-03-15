#pragma once

#include "default.hpp"

#include "gr-base/ecs/entity.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/file/default.hpp"

class GameObject : public Entity
{
public:
    static GameObjectHandle Instantiate(GameObjectHandle other);

    static GameObjectHandle Create(const std::string& name);

    static GameObject* Get(GameObjectHandle handle);

    static void DestroyImediate(GameObjectHandle gameObjectID);

    static void Destroy(GameObjectHandle gameObjectID);

    GameObject() = delete;

    GameObject(GameObjectHandle id);
    GameObject(GameObject&& other) noexcept;
    ~GameObject();

    Transform* GetTransform() const;

    void Copy(GameObjectHandle other);

    inline std::string GetName() const
    {
        return m_name;
    }

    inline void SetName(const std::string& name)
    {
        m_name = name;
    }

    inline void SetScene(Scene* scene)
    {
        m_scene = scene;
    }

    inline Scene* GetScene() const
    {
        return m_scene;
    }

    inline void SetIsStatic(bool value)
    {
        m_isStatic = value;
    }

    inline bool GetIsStatic() const
    {
        return m_isStatic;
    }

    GameObjectHandle FindChildByName(const std::string& name);

private:
    ComponentHandle m_transform;

    std::string m_name;

    Scene* m_scene;

    bool m_isStatic;
};


