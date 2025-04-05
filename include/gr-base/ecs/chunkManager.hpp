#pragma once

#include "default.hpp"

#include "field.hpp"

#include <cstddef>
#include <cstring>
#include <memory>

class Stream;

using ChunkCount = std::size_t;

constexpr const std::size_t CHUNK_SIZE = 1024;

template <typename T, ChunkCount N>
struct ComponentChunk
{
    T components[N];
    bool used[N] = {false};
};

class BaseChunk
{
public:
    BaseChunk();

    bool bCanEnableUpdate;
    
    virtual Component *allocateComponent(Entity *entity, const Component *) = 0;

    virtual void deallocate(Component *component) = 0;

    virtual void allinit() = 0;

    virtual void allupdate() = 0;

    template <typename ...Args>
    bool registerField(const std::string &name, Args &&...args);

    std::vector<Field> getFields() const;

    void (*init)(Component *component);

    void (*start)(Component *component);

    void (*update)(Component *component);

    void (*enable)(Component *component);

    void (*disable)(Component *component);

    bool (*serialize)(Component *component, Stream *stream);

    bool (*deserialize)(Component *component, Stream *stream);

    void (*destroy)(Component *component);

    void (*updateall)(BaseChunk *);

    void (*initall)(BaseChunk *);

    void (*deallocatein)(BaseChunk *, Component *component);

    Component *(*allocatein)(BaseChunk *, Entity *, const Component *component);

private:
    std::unordered_map<std::string, grm::size> m_fieldID;

    std::vector<Field> m_fieldMap;
};

template <typename... Args>
inline bool BaseChunk::registerField(const std::string &name, Args &&...args) {
    auto it = m_fieldID.find(name);
    if (it != m_fieldID.end()) {
        return true;
    }

    auto id = m_fieldMap.size();
    m_fieldMap.emplace_back(name, std::forward<Args>(args)...);
    m_fieldID.emplace(name, id);
    return true;
}

template <typename T, ChunkCount N>
class ChunkManager : public BaseChunk {
public:
    ComponentChunk<T, N>* allocateChunk();
    
    Component* allocateComponent(Entity *, const Component *) override;
    
    void deallocate(Component *) override;
    
    void allinit() override;
    
    void allupdate() override;

private:
    std::vector<std::unique_ptr<ComponentChunk<T, N>>> chunks;
};

template <typename T, ChunkCount N>
ComponentChunk<T, N>* ChunkManager<T, N>::allocateChunk() {
    return &(*chunks.emplace_back(std::make_unique<ComponentChunk<T, N>>()));
}

template <typename T, ChunkCount N>
Component* ChunkManager<T, N>::allocateComponent(Entity *entity, const Component *other) {
    Component *component = nullptr;
    for (auto &chunk : chunks) {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            if (!chunk->used[i]) {
                chunk->used[i] = true;

                component = &chunk->components[i];
                break;
            }
        }
    }

    if (component == nullptr)
    {
        ComponentChunk<T, N>* newChunk = allocateChunk();
        newChunk->used[0] = true;
        
        component = &newChunk->components[0];
    }

    static_cast<T *>(component)->entity = entity;

    if (other != nullptr)
    {
        new (component) T(*static_cast<const T *>(other));
    } else
    {
        new (component) T();
    }

    return component;
}

template <typename T, ChunkCount N>
void ChunkManager<T, N>::deallocate(Component *component)
{
    for (auto &chunk : chunks)
    {
        for (int i = 0; i < CHUNK_SIZE; ++i)
        {
            if (&chunk->components[i] == component)
            {
                std::destroy_at(static_cast<T *>(component));

                chunk->used[i] = false;

                std::memset(component, 0, sizeof(T));
                return;
            }
        }
    }
    std::cerr << "Componente não encontrado para desalocar!\n";
}

template <typename T, ChunkCount N>
void ChunkManager<T, N>::allinit()
{
    for (auto &chunk : chunks)
    {
        for (int i = 0; i < CHUNK_SIZE; ++i)
        {
            if (chunk->used[i])
            {
                init(reinterpret_cast<Component *>(&chunk->components[i]));
            }
        }
    }
}

template <typename T, ChunkCount N>
void ChunkManager<T, N>::allupdate()
{
    if (!bCanEnableUpdate)
    {
        return;
    }
    
    for (auto &chunk : chunks)
    {
        for (int i = 0; i < CHUNK_SIZE; ++i)
        {
            if (chunk->used[i] && chunk->components[i].bCanUseUpdate)
            {
                update(reinterpret_cast<Component *>(&chunk->components[i]));
            }
        }
    }
}



