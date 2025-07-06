#pragma once

#include "default.hpp"

#include "field.hpp"

#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>

#define MAX_COMPONENT_CHUNK 64

class Stream;

template <typename T, size_t C>
struct Chunk
{
    T data[C];
    bool used[C] = {false};
};

class IComponentStorage
{
public:
    IComponentStorage();

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

    void (*updateall)(IComponentStorage *);

    void (*initall)(IComponentStorage *);

    void (*deallocatein)(IComponentStorage *, Component *component);

    Component *(*allocatein)(IComponentStorage *, Entity *, const Component *component);

    std::mutex m_mutex;

private:
    std::unordered_map<std::string, grm::size> m_fieldID;

    std::vector<Field> m_fieldMap;
};

template <typename... Args>
inline bool IComponentStorage::registerField(const std::string &name, Args &&...args) {
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
using ComponentChunk = Chunk<T, MAX_COMPONENT_CHUNK>;

template <typename T>
class ComponentStorage : public IComponentStorage
{
public:
    ComponentStorage() : m_lastchunk(nullptr) {}

    Component* allocateComponent(Entity *, const Component *) override;
    
    void deallocate(Component *) override;
    
    void allinit() override;
    
    void allupdate() override;

private:
    std::vector<std::unique_ptr<ComponentChunk<T>>> m_chunklist;

    std::vector<std::tuple<std::size_t, ComponentChunk<T> *>> m_freeslot;
    
    ComponentChunk<T> *m_lastchunk;
};

template <typename T>
Component* ComponentStorage<T>::allocateComponent(Entity *entity, const Component *other)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_freeslot.empty())
    {
        auto [id, chunk] = m_freeslot.back();
        m_freeslot.pop_back();

        chunk->used[id] = true;

        auto component = static_cast<Component *>(&chunk->data[id]);
        ((T *)component)->entity = entity;

        component = static_cast<Component *>(other == nullptr ? new (component) T() : new (component) T (*static_cast<const T *>(other)));

        return component;
    }

    if (m_lastchunk != nullptr)
    {
        for (std::size_t i={};i<MAX_COMPONENT_CHUNK;i++)
        {
            if (m_lastchunk->used[i])
                continue;

            m_lastchunk->used[i] = true;

            auto component = static_cast<Component *>(&m_lastchunk->data[i]);
            ((T *)component)->entity = entity;

            component = static_cast<Component *>(other == nullptr ? new (component) T() : new (component) T (*static_cast<const T *>(other)));
                
            return component;
        }
    }

    auto chunk = &(*m_chunklist.emplace_back(std::make_unique<ComponentChunk<T>>()));
    chunk->used[0] = true;

    m_lastchunk = chunk;

    auto component = static_cast<Component *>(&chunk->data[0]);
    ((T *)component)->entity = entity;

    component = static_cast<Component *>(other == nullptr ? new (component) T() : new (component) T (*static_cast<const T *>(other)));

    return component;
}

template <typename T>
void ComponentStorage<T>::deallocate(Component *component)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto &chunk : m_chunklist)
    {
        auto base = reinterpret_cast<std::uintptr_t>(chunk->data);
        auto target = reinterpret_cast<std::uintptr_t>(component);

        auto offset = (target - base) / sizeof(T);

        if (target >= base && offset <= MAX_COMPONENT_CHUNK)
        {
            chunk->used[offset] = false;
            std::memset(component, 0, sizeof(T));

            m_freeslot.push_back(std::make_tuple(static_cast<std::size_t>(offset), chunk.get()));
            return;
        }
    }

    std::cerr << "Falha ao desalocar " << typeid(T).name() << ", está fora da stack ou foi liberado mais cedo!!!" << std::endl;
}

template <typename T>
void ComponentStorage<T>::allinit()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto &chunk : m_chunklist)
    {
        for (int i = 0; i < MAX_COMPONENT_CHUNK; ++i)
        {
            if (chunk->used[i])
            {
                lock.unlock();

                init(reinterpret_cast<Component *>(&chunk->data[i]));

                lock.lock();
            }
        }
    }
}

template <typename T>
void ComponentStorage<T>::allupdate()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!bCanEnableUpdate)
    {
        return;
    }
    
    for (auto &chunk : m_chunklist)
    {
        for (int i = 0; i < MAX_COMPONENT_CHUNK; ++i)
        {
            if (chunk->used[i] && chunk->data[i].bCanUseUpdate)
            {
                lock.unlock();

                update(reinterpret_cast<Component *>(&chunk->data[i]));

                lock.lock();
            }
        }
    }
}

#define DEFINE_HAS_OWN_METHOD(METHOD_NAME)                                     \
template<typename T>                                                           \
class has_##METHOD_NAME##_method {                                             \
private:                                                                       \
    template<typename U>                                                       \
    static auto test(U*) -> decltype(                                          \
        std::declval<U>().METHOD_NAME(),                                       \
        std::is_same<decltype(&U::METHOD_NAME), void (U::*)()>{}               \
    );                                                                         \
                                                                               \
    template<typename>                                                         \
    static std::false_type test(...);                                          \
                                                                               \
public:                                                                        \
    static constexpr bool value = decltype(test<T>(nullptr))::value;          \
};


#define DEFINE_HAS_OWN_METHOD_WITH_SIG(METHOD_NAME, RETURN_TYPE, ...)                \
template<typename T>                                                                 \
class has_##METHOD_NAME##_method {                                                   \
private:                                                                             \
    template<typename U>                                                             \
    static auto test(U*) -> decltype(                                                \
        std::is_same<                                                                \
            RETURN_TYPE (U::*)(__VA_ARGS__),                                         \
            decltype(&U::METHOD_NAME)                                                \
        >{}                                                                          \
    );                                                                               \
                                                                                     \
    template<typename>                                                               \
    static std::false_type test(...);                                                \
                                                                                     \
public:                                                                              \
    static constexpr bool value = decltype(test<T>(nullptr))::value;                \
};

class Stream;

DEFINE_HAS_OWN_METHOD(Init)
DEFINE_HAS_OWN_METHOD(Start)
DEFINE_HAS_OWN_METHOD(Update)
DEFINE_HAS_OWN_METHOD(Enable)
DEFINE_HAS_OWN_METHOD(Disable)
DEFINE_HAS_OWN_METHOD(Destroy)

DEFINE_HAS_OWN_METHOD_WITH_SIG(Serialize, bool, Stream *)
DEFINE_HAS_OWN_METHOD_WITH_SIG(Deserialize, bool, Stream *)



