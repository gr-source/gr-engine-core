#pragma once

#include "gr-base/ecs/imethodinvoke.hpp"

#include <type_traits>
#include <utility>

template <typename T>
using ArgAsPointer = std::add_pointer_t<std::remove_reference_t<T>>;

template <typename T, typename R, typename ...Args>
class MethodInvoke : public IMethodInvoke
{
    using MethodPtr = R (T::*)(Args...);

    MethodPtr _method;

public:
    MethodInvoke(MethodPtr method) : _method(method) {}

    void call(void *obj, void **args, void *result) override
    {
        if constexpr (std::is_void<R>::value)
        {
            invokel(static_cast<T *>(obj), _method, args, std::index_sequence_for<Args...>{});
            return;
        } else
        {
            if (result != nullptr)
                *static_cast<R *>(result) = invokel(static_cast<T *>(obj), _method, args, std::index_sequence_for<Args...>{});
            else
                invokel(static_cast<T *>(obj), _method, args, std::index_sequence_for<Args...>{});
        }
    }

private:

    template <std::size_t ...I>
    R invokel(T *obj, MethodPtr ptr, void **args, std::index_sequence<I...>)
    {
        return (obj->*ptr)(*reinterpret_cast<ArgAsPointer<Args>>(args[I])...);
    }
};


