#include "gr-base/task/shadersavetask.hpp"

#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/externals/hash.hpp"
#include <stdexcept>

ShaderSaveTask::ShaderSaveTask(const std::string& path, const ShaderAsset& asset) : m_path(path), m_asset(asset)
{}

void ShaderSaveTask::onPreExecute()
{
    auto filename = gfile::get_file_name(m_path);

    Console::LogInfo("[ShaderSaveTask] Saving %s.", filename.c_str());
}

std::string ShaderSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(hash::fnv1_str(m_path.c_str())));

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
    auto name = m_asset.name;

    if (!result.success)
        return Console::LogError("[ShaderSaveTask] falied in to save asset '%s'!!!", name.c_str());

    AssetHash hash = hash::fnv1_str(m_path.c_str());

    AssetDB::Registry(hash, m_path);

    std::string dst = AssetDB::Resolve(hash) + HEADER_SHADER_EXTENSION;
    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[ShaderSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return AssetDB::Unregistry(hash);
    }

    Console::LogInfo("[ShaderSaveTask] Asset saved sucessfully '%s'", name.c_str());
}


