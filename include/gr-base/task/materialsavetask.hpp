#pragma once

#include "gr-base/asset/materialasset.hpp"

#include "gr-base/core/thread/asynctask.hpp"

class MaterialSaveTask : public IAsyncTask<int>
{
public:
    MaterialSaveTask(const std::string& file, const MaterialAsset& asset);

private:
    MaterialAsset m_asset;

    std::string m_file;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;
};


