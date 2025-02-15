#pragma once

#include "default.hpp"

#include "field.hpp"

#include <memory>

class Stream;

const int CHUNK_SIZE = 500;

template <typename T>
struct ComponentChunk {
    T components[CHUNK_SIZE];
    bool used[CHUNK_SIZE] = {false};
};

class BaseChunk
{
public:
    BaseChunk();
    
    virtual Component *allocateComponent(const Component *) = 0;

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

template <typename T>
class ChunkManager : public BaseChunk {
public:
    ComponentChunk<T>* allocateChunk();
    
    Component* allocateComponent(const Component *) override;
    
    void deallocate(Component *component) override;
    
    void allinit() override;
    
    void allupdate() override;

private:
    std::vector<std::unique_ptr<ComponentChunk<T>>> chunks;
};

template <typename T>
ComponentChunk<T>* ChunkManager<T>::allocateChunk() {
    return &(*chunks.emplace_back(std::make_unique<ComponentChunk<T>>()));
}

template <typename T>
Component* ChunkManager<T>::allocateComponent(const Component *other) {
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
        ComponentChunk<T>* newChunk = allocateChunk();
        newChunk->used[0] = true;
        
        component = &newChunk->components[0];
    }

    if (other != nullptr)
    {
        new (component) T(*static_cast<const T *>(other));
    } else
    {
        new (component) T();
    }

    return component;
}

template <typename T>
void ChunkManager<T>::deallocate(Component *component) {
    for (auto &chunk : chunks) {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            if (&chunk->components[i] == component) {
                chunk->used[i] = false;
                return;
            }
        }
    }
    std::cerr << "Componente não encontrado para desalocar!\n";
}

template <typename T>
void ChunkManager<T>::allinit()
{
    for (auto &chunk : chunks) {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            if (chunk->used[i]) {
                init(reinterpret_cast<Component *>(&chunk->components[i]));
            }
        }
    }
}

template <typename T>
void ChunkManager<T>::allupdate()
{
    for (auto &chunk : chunks) {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            if (chunk->used[i] /*&& reinterpret_cast<Component *>(&chunk->components[i]).activeUpdate*/) {
                update(reinterpret_cast<Component *>(&chunk->components[i]));
            }
        }
    }
}



