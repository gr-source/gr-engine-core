#pragma once

#include "gr-base/asset/URI.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

struct AssetSaveInfo
{
    std::shared_ptr<IAsset> asset;

    AssetType type;

    AssetID id;

    URI uri;
};

class AssetSaveTask : public IAsyncTask<std::string>
{
public:
    AssetSaveTask(const AssetSaveInfo& info);

private:
    const AssetSaveInfo m_info;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};

