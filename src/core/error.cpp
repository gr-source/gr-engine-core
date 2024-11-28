#include "core/error.hpp"

std::string Error::buffer;

std::mutex Error::mutex;

void Error::SetError(const std::string &msg) {
    buffer += "==========";
    buffer += msg;
    buffer += "==========";
}

std::string Error::GetError() {
    auto temp = buffer;
    buffer.clear();
    return temp;
}