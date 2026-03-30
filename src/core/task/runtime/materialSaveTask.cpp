#include "gr-base/core/task/runtime/materialSaveTask.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/file/stream.hpp"

MaterialSaveTask::MaterialSaveTask(const std::string& file, const Material* material) : m_file(file), m_material(material)
{}

// private
int MaterialSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);

    FileHeader header{{'g', 'r', 'm', 't'}, 1.0f};
    if (!stream.write(&header, sizeof(FileHeader)))
        return 2;

    // m_material->serialize(&stream);

    stream.close();

    return 0;
}

void MaterialSaveTask::onPostExecute(int result)
{
    if (result != 0)
        return Console::LogError("[ MaterialSaveTask ] fail to save material: %s", m_file.c_str());

    Console::LogInfo("[ MaterialSaveTask ] %s, saved sucessfully", m_file.c_str());
}


