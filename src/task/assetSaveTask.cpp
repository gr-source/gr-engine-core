#include "gr-base/task/assetSaveTask.hpp"

#include "gr-base/core/assetResolver.hpp"
#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/file/fileStream.hpp"
#include "gr-base/file/file.hpp"

#include <string.h>

AssetSaveTask::AssetSaveTask(const AssetSaveInfo& info) :
    m_info(info)
{}

void AssetSaveTask::onPreExecute()
{
    auto name =
        m_info.uri.toString();

    Console::LogInfo("[ Asset ] Saving %s....", name.c_str());
}

std::string AssetSaveTask::doInBackground()
{
    auto name =
        m_info.uri.toString();

    std::string path =
        gfile::create_temp_file(std::to_string(engine::hash::fnv1_str(name)));

    FileStream stream(path, StreamType::OUTPUT);

    uint64_t sign;
    switch (m_info.type)
    {
        case AssetType_Model:
            sign = HEADER_MODEL_SIGN;
            break;
        case AssetType_Texture:
            sign = HEADER_TEXTURE_SIGN;
            break;
        case AssetType_Mesh:
            sign = HEADER_MESH_SIGN;
            break;
        case AssetType_Material:
            sign = HEADER_MATERIAL_SIGN;
            break;
        case AssetType_Shader:
            sign = HEADER_SHADER_SIGN;
            break;
        default:
            throw std::runtime_error("This asset is not valid");
            break;
    }

    FileHeader header{};
    header.sign = sign;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        throw std::runtime_error("Falied to write header");

    if (!m_info.asset->serialize(&stream))
        throw std::runtime_error("Falied to serialize asset");

    return path;
}

void AssetSaveTask::onPostExecute(Result<std::string> result)
{
    auto name = m_info.uri.toString();

    if (!result.success)
        return Console::LogError("[ Asset ] name='%s' error='%s'.", name.c_str(), result.error.c_str());

    std::string extension = ".unknown";
    switch (m_info.type)
    {
        case AssetType_Model:
            extension = HEADER_MODEL_EXTENSION;
            break;
        case AssetType_Texture:
            extension = HEADER_TEXTURE_EXTENSION;
            break;
        case AssetType_Mesh:
            extension = HEADER_MESH_EXTENSION;
            break;
        case AssetType_Material:
            extension = HEADER_MATERIAL_EXTENSION;
            break;
        case AssetType_Shader:
            extension = HEADER_SHADER_EXTENSION;
            break;
        default:
            break;
    }

    std::string realPath =
        AssetResolver::Resolve(m_info.uri) + extension;

    {
        if (gfile::exists(realPath))
            gfile::remove(realPath);

        std::string parent = gfile::parent(realPath);

        if (!gfile::exists(parent) && !gfile::create_directories(parent))
            return;
    }

    std::string src = result.value;

    if (!gfile::move(src.c_str(), realPath.c_str()))
        return Console::LogError("[ Asset ] Failed to move file 'asset=%s from=%s to=%s'", name.c_str(), src.c_str(), realPath.c_str());

    AssetInfo info;
    info.type = m_info.type;
    info.id = m_info.id;
    info.hash = engine::hash::fnv1_str(name);

    strncpy(info.uri, name.c_str(), sizeof(info.uri));

    AssetDB::Registry(info);

    Console::LogInfo("[ MaterialSaveTask ] Asset saved sucessfully '%s'", name.c_str());
}



