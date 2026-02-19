#pragma once

#include <bitset>
#include <vector>

#include "utils.hpp"

class Entity
{
public:
    Entity(EntityID id);
    virtual ~Entity() {}

    template <typename T, typename... Args>
    T* addComponent(Args&&... args);

    template <typename T>
    T* getComponent() const;

    Behavior* addComponent(const char* name);

    Behavior* getComponent(const char* name) const;

    Behavior* addComponent(uint64_t hash, ComponentHandle source = HANDLE_INVALID);

    Behavior* getComponent(uint64_t hash) const;

    void destroy(uint64_t hash);

    bool reinstance(const char* name, Behavior* dest, const Behavior* src);

    std::vector<uint64_t> GetHashes() const;

    ComponentHandle GetID(uint64_t hash) const;

    template <typename T>
    ComponentHandle GetID() const;

    EntityID getCacheID() const;

    void Init();

    void Start();
    
private:
    typedef struct ComponentLookup
    {
        ComponentHash hash;
        ComponentHandle handle;
    } ComponentLookup;

    ComponentLookup m_lookupTable[COMPONENT_TABLE_SIZE];

    std::bitset<COMPONENT_TABLE_SIZE> m_signature;

    std::vector<uint64_t> m_activeHashes;

    EntityID m_id;
};

template <typename T, typename... Args>
inline T* Entity::addComponent(Args &&...args)
{
    static_assert(sizeof(T) != 0);
    return static_cast<T*>(addComponent(GetHashByType<T>(nullptr)));
}

template <typename T>
inline T* Entity::getComponent() const
{
    static_assert(sizeof(T) != 0);
    return static_cast<T*>(getComponent(GetHashByType<T>(nullptr)));
}

template <typename T>
inline ComponentHandle Entity::GetID() const
{
    return GetID(GetHashByType<T>(nullptr));
}


