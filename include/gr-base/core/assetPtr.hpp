#pragma once

template<typename T, typename IdType>
class AssetPtr
{
private:
    IdType id;
public:
    AssetPtr() = default;
    AssetPtr(IdType id) : id(id) {}

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

    constexpr explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

    constexpr operator IdType() const noexcept
    {
        return id;
    }
};


