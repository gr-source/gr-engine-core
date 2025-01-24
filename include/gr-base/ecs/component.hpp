#pragma once

#include <gr-math/gmath.hpp>

#include <ostream>
#include <istream>

class GameObject;
class Entity;
class Stream;

class Component {
public:
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual bool Serialize(Stream *streaming) { return false; }

    virtual bool Deserialize(Stream *streaming) { return false; }

    virtual void Destroy() {}

    GameObject *getGameObject() const;
    
    Entity* entity;
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
    } \
    inline GameObject *GetGameObject() { \
        return entity->getGameObject(); \
    } \
 
#ifdef REGISTRY
#include "ecs/componentRegistry.hpp"

#define REGISTER_COMPONENT(cls) \
    static bool is_register_##cls = \
        ComponentRegistry::Registry<cls>(); \

#define REGISTER_FIELD(cls, type, field) \
    static bool is_register_##cls##_##type##_##field = \
        ComponentRegistry::GetPool<cls>() != nullptr ? ComponentRegistry::GetPool<cls>()->RegisterField(#field, FieldType_ ##type, reinterpret_cast<std::size_t>(&reinterpret_cast<cls *>(0)->field)) : false;
        
#endif

#ifdef INTERNAL_REGISTER_SCRIPT
#include "ecs/componentRegistry.hpp"

#define REGISTER_COMPONENT(cls) \
    static bool is_register_##cls = \
        ComponentRegistry::Registry<cls>(true); \

#endif


