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

    grr::gTexture *create(const std::string &name);

    void load(const std::string &path, std::function<void (grr::gTexture *)> func);

    bool remove_task(const std::string &name);

    const char *getname(grr::gTexture *texture);

    std::size_t getcount();

private:
    std::unordered_map<std::string, std::shared_ptr<TaskInfo<grr::gTexture*>>> m_texturetaskmap;

    std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> m_texturemap;

    std::unordered_map<grr::gTexture *, std::string> m_texturenamemap;

    std::mutex m_mutex;
};


