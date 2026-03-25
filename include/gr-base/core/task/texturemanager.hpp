#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "runtime/default.hpp"

#include <gr-render/gTexture.h>

extern "C"
{
#include "gr-base/externals/array.h"
}

class TextureManager
{
public:
    TextureManager() = default;
    ~TextureManager() = default;

    gr::gTexture *create(const std::string &name);

    void load(const std::string &path, std::function<void (gr::gTexture *)> func);

    bool remove_task(const std::string &name);

    const char *getname(gr::gTexture *texture);

    std::size_t getcount();

private:
    std::unordered_map<std::string, std::shared_ptr<TaskInfo<gr::gTexture*>>> m_texturetaskmap;

    std::unordered_map<std::string, std::unique_ptr<gr::gTexture>> m_texturemap;

    std::unordered_map<gr::gTexture *, std::string> m_texturenamemap;

    std::mutex m_mutex;
};


