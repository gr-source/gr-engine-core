#pragma once

#include "gr-base/asset/modelasset.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

class ModelSaveTask : public IAsyncTask<std::string>
{
public:
    static ModelAsset ConvertToAssetModel(Scene* scene, Entity entity);

    ModelSaveTask(const std::string& path, const ModelAsset& asset);
    ModelSaveTask(const std::string& path, Scene* scene, Entity entity);

private:
    const ModelAsset m_asset;

    const std::string m_path;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


