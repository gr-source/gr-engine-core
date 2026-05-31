#include "gr-base/task/materialsavetask.hpp"

#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/file/stream.hpp"
#include "gr-base/externals/hash.hpp"

#include <gr-base/core/file/fileStream.hpp>

MaterialSaveTask::MaterialSaveTask(const std::string& path, const MaterialAsset& asset) : m_path(path), m_asset(asset)
{}

// private
void MaterialSaveTask::onPreExecute()
{
    auto name = m_asset.name;

    Console::LogError("[MaterialSaveTask] Saving asset '%s' in '%s'", name.c_str(), name.c_str());
}

std::string MaterialSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(hash::fnv1_str(m_path.c_str())));

    FileStream stream(file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_MATERIAL_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        throw std::runtime_error("Falied to write header");

    if (!m_asset.serialize(&stream))
        throw std::runtime_error("Falied to serialize asset");

    stream.close();

    return file;
}

void MaterialSaveTask::onPostExecute(Result<std::string> result)
{
    auto name = m_asset.name;

    if (!result.success)
        return Console::LogError("[MaterialSaveTask] falied in to save asset '%s'!!!", name.c_str());

    AssetHash hash = hash::fnv1_str(m_path.c_str());

    AssetDB::Registry(hash, m_path);

    std::string dst = AssetDB::Resolve(hash) + HEADER_MATERIAL_EXTENSION;
    if (gfile::exists(dst))
        gfile::remove(dst);

    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[MaterialSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return AssetDB::Unregistry(hash);
    }

    Console::LogInfo("[MaterialSaveTask] Asset saved sucessfully '%s'", name.c_str());
}


