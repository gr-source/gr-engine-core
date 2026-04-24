#include "gr-base/task/meshsavetask.hpp"

#include "gr-base/core/assetReference.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/file/fileStream.hpp"

MeshSaveTask::MeshSaveTask(const std::string& file, const MeshAsset& asset) : m_file(file), m_asset(asset)
{}

// private
void MeshSaveTask::onPreExecute()
{
    auto name = gfile::getname(m_file.c_str());

    Console::LogError("[Task] Saving mesh '%s'", name);
}

int MeshSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_MESH_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();

    return 0;
}

void MeshSaveTask::onPostExecute(int result)
{
    auto name = gfile::getname(m_file.c_str());

    if (result != 0)
        return Console::LogError("[Task] falied in to save mesh '%s'!!!", name);

    Console::LogInfo("[Task] Mesh saved sucessfully '%s'", name);

    AssetReference::Registry(m_asset.GetName(), m_file);
}




