#include "gr-base/core/task/runtime/meshSaveTask.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/managers/assetsManager.hpp"

MeshSaveTask::MeshSaveTask(const std::string& file, MeshID id, std::function<void (MeshID)> func) : m_file(file), m_meshID(id), m_func(func)
{}

// private
MeshID MeshSaveTask::doInBackground()
{
    auto meshName = AssetsManager::GetMeshName(m_meshID);

    Stream stream(gfile::get_cache_directory() + "/" + (meshName + ".gmsh"), std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'm', 's', 'h'}, 1.0f, 0};
        if (!stream.writeS(&header, 1, false))
            return ID_INVALID;
    }

    if (!stream.write(meshName))
        return ID_INVALID;

    Mesh* mesh = AssetsManager::GetMesh(m_meshID);
    mesh->serialize(&stream);

    stream.close();

    return m_meshID;
}

void MeshSaveTask::onPostExecute(MeshID result)
{
    auto meshName = AssetsManager::GetMeshName(m_meshID);

    if (result == ID_INVALID)
        return Console::LogError("[ MeshSaveTask ] fail to write mesh %s!", meshName.c_str());

    Console::LogInfo("[ MeshSaveTask ] Writed %s...", meshName.c_str());
        
    std::string temp = gfile::get_cache_directory() + "/" + (meshName + ".gmsh");

    gfile::move(temp.c_str(), m_file.c_str());

    if (m_func != nullptr)
        m_func(result);
}



