#include "core/file/stream.hpp"

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

void Stream::flush() {
    for (auto &&[data, size] : m_data) {
        std::fstream::write(reinterpret_cast<const char *>(data), size);

        std::free(data);
    }
    m_data.clear();
}

bool Stream::write(const void *data, std::size_t size) {
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

bool Stream::read(char *data, std::streamsize size) {
    if (std::fstream::read(reinterpret_cast<char *>(data), size)) {
        return true;
    }
    return false;
}

bool Stream::is_open() const {
    return std::fstream::is_open();
}

std::streampos Stream::tellg() {
    return std::fstream::tellg();
}

std::istream &Stream::seekg(std::streamoff position, std::ios_base::seekdir mode) {
    return std::fstream::seekg(position, mode);
}

void Stream::close() {
    for (auto &&[data, size] : m_data) {
        std::free(data);
    }
    
    m_data.clear();

    std::fstream::close();
}

std::string Stream::getPath() const {
    return m_path;
}
