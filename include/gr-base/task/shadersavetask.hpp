#pragma once

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include <string>

class ShaderSaveTask : public IAsyncTask<std::string>
{
public:
    ShaderSaveTask(const std::string& path, const ShaderAsset& asset);

private:
    const ShaderAsset m_asset;

    const std::string m_path;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


