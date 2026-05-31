#include "gr-base/core/file/fileStream.hpp"

FileStream::FileStream(const std::string& path, StreamType type) : stream(path, (type == StreamType::OUTPUT ? std::ios_base::out : std::ios_base::in) | std::ios_base::binary)
{}

FileStream::~FileStream()
{
    close();
}

bool FileStream::write(const void* buffer, size_t size)
{
    return stream.write(static_cast<const char*>(buffer), size).good();
}

bool FileStream::read(void* buffer, size_t size)
{
    return stream.read(static_cast<char*>(buffer), size).good();
}

bool FileStream::seekg(size_t pos)
{
    stream.seekg(pos, std::ios::beg);
    return stream.good();
}

bool FileStream::seekp(size_t pos)
{
    stream.seekp(pos, std::ios::beg);
    return stream.good();
}

size_t FileStream::tellg()
{
    return stream.tellg();
}

size_t FileStream::tellp()
{
    return stream.tellp();
}

bool FileStream::eof() const
{
    return stream.eof();
}

void* FileStream::data() const
{
    return nullptr;
}

void FileStream::close()
{
    stream.close();
}
