#include "gr-base/task/textureBuildTask.hpp"

#include "gr-base/asset/textureAsset.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/assetDB.hpp"

#include "gr-base/managers/taskmanager.hpp"

#include "gr-base/task/assetSaveTask.hpp"

#include "gr-base/file/file.hpp"

#include <stdexcept>
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

    // stbi_set_flip_vertically_on_load_thread(true);

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
    auto name = m_uri.toString();

    if (!result.success)
        return Console::LogError("[ TextureBuildTask ] erro in build texture name='%s' error='%s'.", name.c_str(), result.error.c_str());

    Console::LogInfo("[ TextureBuildTask ] Texture '%s' build sucessfully!", name.c_str());

    AssetID assetID = AssetDB::FindByHash(engine::hash::fnv1_str(name));
    if (assetID == INVALID_ASSET)
        assetID = AssetDB::CreateID();

    AssetSaveInfo info;
    info.uri = m_uri;
    info.id = assetID;
    info.type = AssetType_Texture;
    info.asset = std::make_shared<TextureAsset>(result.value);

    TaskManager::Add<AssetSaveTask>(info);
}

