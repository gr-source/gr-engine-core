#include "gr-base/task/materialsavetask.hpp"

#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/asset/assetDB.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/file/stream.hpp"
#include "gr-base/externals/hash.hpp"

#include <gr-base/core/file/fileStream.hpp>

MaterialSaveTask::MaterialSaveTask(const URI& uri, const MaterialAsset& asset) : m_uri(uri), m_asset(asset)
{}

// private
void MaterialSaveTask::onPreExecute()
{
    auto name = m_uri.toString();

    Console::LogInfo("[ MaterialSaveTask ] Saving asset '%s'.", name.c_str());
}

std::string MaterialSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(m_uri.toString())));

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
    /*
    auto name = m_uri.toString();

    if (!result.success)
        return Console::LogError("[ MaterialSaveTask ] falied in to save name='%s' error='%s'.", name.c_str(), result.error.c_str());

    std::string dst = AssetResolver::Resolve(m_uri) + HEADER_MATERIAL_EXTENSION;
    if (gfile::exists(dst))
        gfile::remove(dst);

    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[ MaterialSaveTask ] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return;
    }

    AssetDB::Registry(name, AssetFlags_AssetMaterial);

    Console::LogInfo("[ MaterialSaveTask ] Asset saved sucessfully '%s'", name.c_str());
    */
}


