#pragma once

#include "gr-base/asset/materialasset.hpp"

#include "gr-base/core/thread/asynctask.hpp"

class MaterialSaveTask : public IAsyncTask<std::string>
{
public:
    MaterialSaveTask(const std::string& path, const MaterialAsset& asset);

private:
    const MaterialAsset m_asset;

    const std::string m_path;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


