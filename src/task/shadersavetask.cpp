#include "gr-base/task/shadersavetask.hpp"

#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/asset/assetDB.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"

#include <stdexcept>

ShaderSaveTask::ShaderSaveTask(const URI& uri, const ShaderAsset& asset) : m_uri(uri), m_asset(asset)
{}

void ShaderSaveTask::onPreExecute()
{
    auto name = m_uri.toString();

    Console::LogInfo("[ShaderSaveTask] Saving %s.", name.c_str());
}

std::string ShaderSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(m_uri.toString())));

    FileStream stream(file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_SHADER_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        throw std::runtime_error("Falied to write header");

    if (!m_asset.serialize(&stream))
        throw std::runtime_error("Falied to serialize asset");

    stream.close();

    return file;
}

void ShaderSaveTask::onPostExecute(Result<std::string> result)
{
    auto name = m_uri.toString();

    if (!result.success)
        return Console::LogError("[ShaderSaveTask] falied in to save asset '%s'!!!", name.c_str());

    AssetHash hash = engine::hash::fnv1_str(name);

    std::string dst = AssetResolver::Resolve(m_uri) + HEADER_SHADER_EXTENSION;
    std::string src = result.value;

    if (gfile::exists(dst))
        gfile::remove(dst);

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[ShaderSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());
        return;
    }

    AssetDB::Registry(hash, name);

    Console::LogInfo("[ShaderSaveTask] Asset saved sucessfully '%s'", name.c_str());
}


