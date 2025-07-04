#pragma once

#include <type_traits>

class IMethodInvoke
{
public:
    virtual ~IMethodInvoke() {}

    virtual void call(void *obj, void **args, void *result) = 0;

    template <typename R, typename ...Args>
    R invoke(void *obj, Args &&...args)
    {
        void *argv[] = {(void *)&args...};
        if constexpr (std::is_void<R>::value)
        {
            call(obj, argv, nullptr);
        } else
        {
            R result;

            call(obj, argv, &result);

            return result;
        }
    }
};


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


