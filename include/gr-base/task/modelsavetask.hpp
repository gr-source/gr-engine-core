#pragma once

#include "gr-base/asset/modelasset.hpp"
#include "gr-base/core/thread/asynctask.hpp"
#include "gr-base/ecs/default.hpp"

class ModelSaveTask : public IAsyncTask<int>
{
public:
    static ModelAsset ConvertToAssetModel(Handle handle);

    ModelSaveTask(const std::string& file, const ModelAsset& asset);
    ModelSaveTask(const std::string& file, Handle handle);

private:
    std::string m_file;

    ModelAsset m_asset;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;
};


