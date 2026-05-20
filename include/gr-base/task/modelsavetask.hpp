#pragma once

#include "gr-base/asset/modelasset.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

class ModelSaveTask : public IAsyncTask<int>
{
public:
    static ModelAsset ConvertToAssetModel(Scene* scene, Entity entity);

    ModelSaveTask(const std::string& file, const ModelAsset& asset);
    ModelSaveTask(const std::string& file, Scene* scene, Entity entity);

private:
    std::string m_file;

    ModelAsset m_asset;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;
};


