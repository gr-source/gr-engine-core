#pragma once

#include "gr-base/file/stream.hpp"

class MemoryStream : public Stream
{
public:
    MemoryStream(uint8_t* data, size_t size);

    MemoryStream(size_t size = 1024);

    bool write(const void* dst, size_t size) override;

    bool read(void* dst, size_t size) override;

    bool seekg(size_t pos) override;

    bool seekp(size_t pos) override;

    size_t tellg() override;

    size_t tellp() override;

    bool eof() const override;

    void* data() const override;

    void close() override;

private:
    uint8_t* begin;
    uint8_t* current;
    uint8_t* end;
    size_t capacity;
};


