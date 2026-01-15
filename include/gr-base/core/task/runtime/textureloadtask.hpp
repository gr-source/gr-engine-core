#pragma once

#include <gr-render/gRender.h>

#include "default.hpp"
#include "gr-base/core/thread/asynctask.hpp"

enum class TextureLoadFlags {};

using TextureLoadFlags_ = uint8_t;

using TextureFunc = std::function<void (grr::gTexture *)>;

class TextureLoadTask : public IAsyncTask<grr::gTexture*>
{
public:
    TextureLoadTask(const std::string &path, TextureFunc func);

    void addFunction(TextureFunc func);

    void invoke(grr::gTexture *texture);

private:
    std::vector<TextureFunc> m_functions;

    std::unique_ptr<char> m_pixels;

    std::string m_path;

protected:
    TextureFormat format;

    int width;

    int height;

    grr::gTexture* doInBackground() override;

    void onPostExecute(grr::gTexture* texture) override;
};




