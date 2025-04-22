#pragma once

#include "taskStatus.hpp"

#include <gr-render/gRender.h>

enum class TextureLoadFlags {};

using TextureLoadFlags_ = uint8_t;

using TextureFunc = std::function<void (grr::gTexture *)>;

class TextureTask {
public:
    TextureTask(const std::string &path, TextureFunc func);

    void addFunction(TextureFunc func);

    void invoke(grr::gTexture *texture);

private:
    std::vector<TextureFunc> m_functions;

    std::unique_ptr<char> m_pixels;

    void upload(std::string path, int width, int height, TextureFormat format);

protected:
    void doInBackground(std::string path);
};




