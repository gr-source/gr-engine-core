#pragma once

#include "gr-base/core/file/stream.hpp"
#include <fstream>

class FileStream : public Stream
{
public:
    FileStream(const std::string& path, StreamType type);
    ~FileStream() override;

    bool write(const void* buffer, size_t size) override;
    
    bool read(void* buffer, size_t size) override;

    bool seekg(size_t pos) override;

    bool seekp(size_t pos) override;

    size_t tellg() override;

    size_t tellp() override;

    bool eof() const override;

    void* data() const override;

    void close() override;

private:
    std::fstream stream;
};
