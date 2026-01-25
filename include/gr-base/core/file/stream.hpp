#pragma once

#include <stdlib.h>
#include <stdint.h>

#include <type_traits>
#include <string>

enum class StreamType
{
    OUTPUT,
    INPUT
};

class Stream
{
public:
    virtual ~Stream() {}

    virtual bool write(const void* buffer, size_t size) = 0;

    virtual bool read(void* buffer, size_t size) = 0;

    virtual bool seekg(size_t pos) = 0;

    virtual bool seekp(size_t pos) = 0;

    virtual size_t tellg() = 0;

    virtual size_t tellp() = 0;

    virtual bool eof() const = 0;

    virtual void* data() const { return nullptr; }

    virtual void close() = 0;

    template <typename T>
    inline bool write(const T& in)
    {
        static_assert(std::is_trivially_copyable_v<T>, "writeStruct only supports POD types");
    
        return write(&in, sizeof(T));
    }

    template <typename T>
    inline bool read(T& out)
    {
        static_assert(std::is_trivially_copyable_v<T>, "writeStruct only supports POD types");

        return read(&out, sizeof(T));
    }
};

template <>
inline bool Stream::write(const std::string& in)
{
    size_t length = static_cast<size_t>(in.length());
    if (!write(length))
        return false;

    return write(in.data(), length * sizeof(char));
}

template <>
inline bool Stream::read(std::string& in)
{
    size_t length;
    if (!read(length))
        return false;

    in.resize(length, '\0');

    return read(in.data(), length);
}


