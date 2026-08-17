#pragma once

#include <cstdint>

template <typename T, typename IdType = uint32_t>
struct ObjectPtr
{
private:
    IdType id = 0;

public:
    ObjectPtr() = default;
    explicit ObjectPtr(IdType id)
        : id(id)
    {
    }

    IdType handle() const
    {
        return id;
    }

    T* get();
    const T* get() const;

    T* operator->()
    {
        return get();
    }

    const T* operator->() const
    {
        return get();
    }

    T& operator*()
    {
        return *get();
    }

    const T& operator*() const
    {
        return *get();
    }

    explicit operator bool() const
    {
        return get() != nullptr;
    }
};

