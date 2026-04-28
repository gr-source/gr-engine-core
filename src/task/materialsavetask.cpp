#include "gr-base/task/materialsavetask.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/file/stream.hpp"

#include <gr-base/core/file/fileStream.hpp>

MaterialSaveTask::MaterialSaveTask(const std::string& file, const MaterialAsset& asset) : m_file(file), m_asset(asset)
{}

// private
void MaterialSaveTask::onPreExecute()
{
    auto name = gfile::get_file_name(m_file.c_str());

    Console::LogInfo("[Task] Saving material '%s'...", name.c_str());
}

int MaterialSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);
    
    FileHeader header{};
    header.sign = HEADER_MATERIAL_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();
    return 0;
}

void MaterialSaveTask::onPostExecute(int result)
{
    auto name = gfile::get_file_name(m_file.c_str());

    if (result != 0)
    {
        Console::LogError("[Task] fail in to save material '%s'!!!", name.c_str());

        if (gfile::exists(m_file))
            gfile::remove(m_file);

        return;
    }

    Console::LogInfo("[Task] Material '%s' saved in '%s'.", name.c_str(), m_file.c_str());
}


