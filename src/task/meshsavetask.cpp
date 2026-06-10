#include "gr-base/task/meshsavetask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/externals/hash.hpp"
#include <string>

MeshSaveTask::MeshSaveTask(const std::string& path, const MeshAsset& asset) : m_path(path), m_asset(asset)
{}

// private
void MeshSaveTask::onPreExecute()
{
    auto name = m_asset.name;

    Console::LogInfo("[MeshSaveTask] Saving asset '%s' in '%s'", name.c_str(), m_path.c_str());
}

std::string MeshSaveTask::doInBackground()
{
    std::string file = gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(m_path.c_str())));

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
    auto name = m_asset.name;

    if (!result.success)
        return Console::LogError("[MeshSaveTask] falied in to save asset '%s'!!!", name.c_str());

    AssetHash hash = engine::hash::fnv1_str(m_path.c_str());

    AssetDB::Registry(hash, m_path);

    std::string dst = AssetDB::Resolve(hash) + HEADER_MESH_EXTENSION;
    if (gfile::exists(dst))
        gfile::remove(dst);

    std::string src = result.value;

    if (!gfile::move(src.c_str(), dst.c_str()))
    {
        Console::LogError("[MeshSaveTask] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), dst.c_str());

        return AssetDB::Unregistry(hash);
    }

    Console::LogInfo("[MeshSaveTask] Asset saved sucessfully '%s'", name.c_str());
}




