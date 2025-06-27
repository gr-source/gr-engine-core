#include "gr-base/core/file/stream.hpp"
#include <fstream>

Stream::Stream(const std::string &path, std::ios_base::openmode mode) : m_path(path), std::fstream(path, mode) {}

Stream::Stream() : std::fstream() { }

Stream::~Stream() {
    Stream::close();
}

std::streamsize Stream::getDataSize() {
    std::streamsize buffer_size = 0;
    for (auto &&[data, size] : m_data) {
        buffer_size += size;
    }
    return buffer_size;
}

void Stream::flush()
{
    for (auto &&[data, size] : m_data)
    {
        std::fstream::write(reinterpret_cast<const char *>(data), size);

        std::free(data);
    }
    m_data.clear();
}

// ********** write template ********** //
template <>
bool Stream::write(const std::string &value)
{
    auto len = value.length();
    if (!write(&len, sizeof(std::size_t)))
    {
        return false;
    }

    return write(value.c_str(), len);
}

template <>
bool Stream::write(const bool &value)
{
    return write(&value, sizeof(bool));
}

template <>
bool Stream::write(const std::size_t &value)
{
    return write(&value, sizeof(std::size_t));
}

bool Stream::write(const void *data, std::size_t size)
{
    if (!data || size <= 0) {
        return false;
    }
    auto ptr = std::malloc(size);

    if (!std::memcpy(ptr, data, size)) {
        return false;
    }

    m_data.emplace_back(ptr, static_cast<std::streamsize>(size));

    return true;
}

// ********** write template ********** //
template <>
bool Stream::read(std::string &value)
{
    std::size_t len = {};
    if (!read(reinterpret_cast<char *>(&len), sizeof(std::size_t)))
    {
        return false;
    }
    value.resize(len, '\0');

    return read(value.data(), len);
}

template <>
bool Stream::read(bool &value)
{
    return read(reinterpret_cast<char *>(&value), sizeof(bool));
}

template <>
bool Stream::read(std::size_t &value)
{
    return read(reinterpret_cast<char *>(&value), sizeof(std::size_t));
}

bool Stream::read(char *data, std::streamsize size)
{
    std::fstream::read(data, size);

    return !std::fstream::fail();
}

bool Stream::is_open() const
{
    return std::fstream::is_open();
}

std::streampos Stream::tellg()
{
    return std::fstream::tellg();
}

std::istream &Stream::seekg(std::streamoff position, std::ios_base::seekdir mode)
{
    return std::fstream::seekg(position, mode);
}

void Stream::close()
{
    flush();

    std::fstream::close();
}

std::string Stream::getPath() const {
    return m_path;
}
