#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/asset/assetDB.hpp"

#include "gr-base/task/texureSaveTask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"

#include <string>

TextureSaveTask::TextureSaveTask(const URI& uri, const TextureAsset& asset) : m_uri(uri), m_asset(asset)
{}

// private
void TextureSaveTask::onPreExecute()
{
    auto name = m_uri.toString();

    Console::LogInfo("[ TextureSaveTask ] Saving asset '%s'.", name.c_str());
}

std::string TextureSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(m_uri.path)));

    FileStream stream(file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_TEXTURE_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        throw std::runtime_error("Falied to write header");

    if (!m_asset.serialize(&stream))
        throw std::runtime_error("Falied to serialize asset");

    stream.close();

    return file;
}

void TextureSaveTask::onPostExecute(Result<std::string> result)
{
    auto name = m_uri.toString();

    if (!result.success)
        return Console::LogError("[ TextureSaveTask ] falied in to save asset name='%s' error='%s'.", name.c_str(), result.error.c_str());

    std::string realPath = AssetResolver::Resolve(m_uri);
    {
        if (gfile::exists(realPath) && !gfile::is_directory(realPath))
            gfile::remove(realPath);

        std::string parent = gfile::parent(realPath);

        if (!gfile::exists(parent) && !gfile::create_directories(parent))
            return;
    }

    std::string dst = realPath + HEADER_TEXTURE_EXTENSION;
    if (gfile::exists(dst))
        gfile::remove(dst);

    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[TextureSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return;
    }

    AssetDB::Registry(name, AssetFlags_AssetTexture);

    Console::LogInfo("[TextureSaveTask] Asset saved sucessfully '%s'", name.c_str());
}




