#pragma once

#include <gr-math/gmath.hpp>

#include <ostream>
#include <istream>

class GameObject;
class Entity;
class Stream;

class Component {
public:
    Entity* entity;

    bool bCanUseUpdate;

    Component();
    virtual ~Component() {}

    virtual void onInit() {}

    virtual void onStart() {}

    virtual void onUpdate() {}

    virtual void onEnable() {}

    virtual void onDisable() {}

    virtual bool onSerialize(Stream *stream) { return false; }

    virtual bool onDeserialize(Stream *stream) { return false; }

    virtual void onDestroy() {}

    virtual void onRelease() {}

    GameObject *getGameObject() const;
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
    template <typename T> \
    inline T *GetComponentInChildren() { return entity->getComponentInChildren<T>(); } \
    inline GameObject *GetGameObject() { \
        return entity->getGameObject(); \
    } \
 
#ifdef REGISTRY
#include "gr-base/ecs/componentRegistry.hpp"

#define REGISTER_COMPONENT(klass) \
    static bool is_register_##klass = \
        ComponentRegistry::Register<klass>(#klass, false); \

#define REGISTER_FIELD(klass, type, field) \
static bool is_register_##klass##_##type##_##field = \
    ComponentRegistry::Find(#klass) != nullptr ? ComponentRegistry::Find(#klass)->registerField(#field, FieldType_ ##type, reinterpret_cast<std::size_t>(&reinterpret_cast<klass *>(0)->field)) : false;


        
#endif

#ifdef INTERNAL_REGISTER_SCRIPT
#include "gr-base/ecs/componentRegistry.hpp"

#define REGISTER_COMPONENT(klass) \
    static bool is_register_##klass = \
        ComponentRegistry::Register<klass>(#klass, true); \

#endif


