#include "gr-base/core/task/runtime/meshLoadTask.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/managers/assetsManager.hpp"

MeshLoadTask::MeshLoadTask(const std::string& file, std::function<void (MeshID)> func) : m_file(file), m_func(func)
{}

// private
MeshID MeshLoadTask::doInBackground()
{
    /*
    Stream stream(m_file, std::ios_base::in | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'm', 's', 'h'}, 1.0f, 0};
        if (!stream.read(header))
            return ID_INVALID;

        if (strncmp(header.sign, "gmsh", 4) != 0)
            return ID_INVALID;
    }

    std::string meshName;
    if (!stream.read(meshName))
        return ID_INVALID;

    MeshID meshID = AssetsManager::FindMesh(meshName);
    if (meshID == ID_INVALID)
    {
        meshID = AssetsManager::CreateMesh(meshName);

        Mesh* mesh = AssetsManager::GetMesh(meshID);
        mesh->deserialize(&stream);
    }

    stream.close();

    return meshID;
    */
    return ID_INVALID;
}

void MeshLoadTask::onPostExecute(MeshID result)
{
    if (result == ID_INVALID)
        return Console::LogError("[ MeshLoadTask ] fail to load mesh %s!", m_file.c_str());

    auto meshName = AssetsManager::GetMeshName(result);

    Console::LogInfo("[ MeshLoadTask ] Loaded %s...", meshName.c_str());

    if (m_func != nullptr)
        m_func(result);
}




