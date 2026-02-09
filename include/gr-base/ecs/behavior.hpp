#pragma once

#include "gr-base/ecs/utils.hpp"

#include <stdint.h>

class GameObject;
class Entity;
class Stream;

class Behavior
{
public:
    EntityID entityID;

    inline Behavior() {}
    virtual ~Behavior() {}

    GameObject* GetGameObject() const;

    Entity* GetEntity() const;

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
class BehaviorImpl : public Behavior
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
        return entity->addComponent<T>(); \
    } \
    template <typename T> \
    inline T *GetComponent() { \
        return entity->getComponent<T>(); \
    } \
    template <typename T> \
    inline bool HasComponent() const { \
        return entity->hasComponent<T>(); \
    }


#include "gr-base/ecs/behaviorRegistry.hpp"

#define CLASS_TO_HASH(k) k##Hash

// static_assert(std::is_base_of<BehaviorImpl<k>, k>::value, "Esta classe precisa ser herdade de BehaviorImpl<T>");
#define REGISTER_COMPONENT(k, p) \
    inline static uint64_t k##Hash = GetHashByType<k>(#k); \
    inline static ComponentInfo k##Info = BehaviorRegistry::Registry<k>(#k, k##Hash, p);









