#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/utils.hpp"

class GameObject;
class Entity;
class Stream;

class Component
{
public:
    GameObjectHandle cacheID;

    bool bCanEnableUpdate;

    Component() = default;
    virtual ~Component() {}

    GameObject* GetGameObject() const;

    template <typename T>
    T* GetObject() const;

    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Destroy() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual bool Serialize(Stream* stream) const { return false; }

    virtual bool Deserialize(Stream* stream) { return false; }

    virtual uint64_t GetHash() const { return 0; };
};

template <typename T>
inline T* Component::GetObject() const
{
    return static_cast<T*>(GetGameObject());
}

template <typename T>
class ComponentImpl : public Component
{
public:
    inline uint64_t GetHash() const override
    {
        static uint64_t hash = GetHashByType<T>("nill");
        return hash;
    }
};

#define CLASS_FUNCTIONS \
    template <typename T> \
    inline T *AddComponent() { \
        return GetObject<Entity>()->addComponent<T>(); \
    } \
    template <typename T> \
    inline T *GetComponent() { \
        return GetObject<Entity>()->getComponent<T>(); \
    }

#include "gr-base/ecs/componentregistry.hpp"

#define CLASS_TO_HASH(k) k##Hash

// static_assert(std::is_base_of<BehaviorImpl<k>, k>::value, "Esta classe precisa ser herdade de BehaviorImpl<T>");
#define REGISTER_COMPONENT(k, p) \
    inline static uint64_t k##Hash = GetHashByType<k>(#k); \
    inline static ComponentInfo k##Info = ComponentRegistry::Registry<k>(#k, k##Hash, p);









