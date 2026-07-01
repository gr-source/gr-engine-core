#include "gr-base/task/meshsavetask.hpp"

#include "gr-base/asset/assetResolver.hpp"
#include "gr-base/asset/assetDB.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/externals/hash.hpp"
#include <string>

MeshSaveTask::MeshSaveTask(const URI& uri, const MeshAsset& asset) : m_uri(uri), m_asset(asset)
{}

// private
void MeshSaveTask::onPreExecute()
{
    auto name = m_uri.toString();

    Console::LogInfo("[ MeshSaveTask ] Saving asset '%s'.", name.c_str());
}

std::string MeshSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(m_uri.toString())));

    FileStream stream(file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_MESH_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        throw std::runtime_error("Falied to write header");

    if (!m_asset.serialize(&stream))
        throw std::runtime_error("Falied to serialize asset");

    stream.close();

    return file;
}

void MeshSaveTask::onPostExecute(Result<std::string> result)
{
    auto name = m_uri.toString();

    if (!result.success)
        return Console::LogError("[ MeshSaveTask ] falied in to save name='%s' error='%s'.", name.c_str(), result.error.c_str());

    AssetHash hash = engine::hash::fnv1_str(name);

    std::string dst = AssetResolver::Resolve(m_uri) + HEADER_MESH_EXTENSION;
    if (gfile::exists(dst))
        gfile::remove(dst);

    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[ MeshSaveTask ] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return;
    }

    AssetDB::Registry(hash, name);

    Console::LogInfo("[MeshSaveTask] Asset saved sucessfully '%s'", name.c_str());
}




