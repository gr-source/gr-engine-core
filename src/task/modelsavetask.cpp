#include "gr-base/task/modelsavetask.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/file/fileStream.hpp"

ModelSaveTask::ModelSaveTask(const std::string& file, const ModelAsset& asset) : m_file(file), m_asset(asset)
{}

void ModelSaveTask::onPreExecute()
{
    const char* name = gfile::getname(m_file.c_str());

    Console::LogInfo("[Task] Saving '%s' model...", name);
}

// private
int ModelSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);

    FileHeader header{};
    header.sign = HEADER_MODEL_SIGN;
    header.version = HEADER_ASSET_VERSION;

    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();
    return 0;
}

void ModelSaveTask::onPostExecute(int result)
{
    const char* name = gfile::getname(m_file.c_str());

    if (result != 0)
        return Console::LogError("[Task] Falied in to save '%s'!!!", name);

    Console::LogInfo("[Taks] Model '%s' saved in '%s'.", name, m_file.c_str());
}


