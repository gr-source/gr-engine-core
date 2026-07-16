#include "gr-base/task/textureBuildTask.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/file.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/task/texureSaveTask.hpp"

#include <vector>
#include <cerrno>

#define STB_IMAGE_IMPLEMENTATION
#include "gr-base/core/stb/stb_image.h"

TextureBuildTask::TextureBuildTask(const std::string& file, const URI& uri) : m_file(file), m_uri(uri)
{}

void TextureBuildTask::onPreExecute()
{
    auto name = gfile::get_file_name(m_uri.path);

    Console::LogInfo("[ TextureBuildTask ] Building texture '%s'.", name.c_str());
}

TextureAsset TextureBuildTask::doInBackground()
{
    if (!gfile::exists(m_file))
        throw std::runtime_error(m_file + ": " + strerror(ENOENT));

    bool isHDR = stbi_is_hdr(m_file.c_str());

    int width;
    int height;
    int ch;

    void* raw = isHDR ?
        static_cast<void*>(stbi_loadf(m_file.c_str(), &width, &height, &ch, 0)) :
        static_cast<void*>(stbi_load(m_file.c_str(), &width, &height, &ch, 0));

    if (raw == nullptr)
        throw std::runtime_error("Failed to load texture: " + m_file);

    size_t pixelCount = width * height * ch;
    size_t byteSize = isHDR
        ? pixelCount * sizeof(float)
        : pixelCount * sizeof(unsigned char);

    std::vector<uint8_t> pixels(byteSize);
    memcpy(pixels.data(), raw, byteSize);

    stbi_image_free(raw);

    TextureAsset asset;
    asset.width = width;
    asset.height = height;
    asset.channels = ch;
    asset.hdr = isHDR;

    asset.raw = pixels;

    return asset;
}

void TextureBuildTask::onPostExecute(Result<TextureAsset> result)
{
    auto name = gfile::get_file_name(m_uri.path);

    if (!result.success)
        return Console::LogError("[ TextureBuildTask ] erro in build texture name='%s' error='%s'.", name.c_str(), result.error.c_str());

    Console::LogInfo("[ TextureBuildTask ] Texture '%s' build sucessfully!", name.c_str());

    TaskManager::Add<TextureSaveTask>(m_uri, result.value);
}

