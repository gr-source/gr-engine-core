#pragma once

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/URI.hpp"

#include "gr-base/core/thread/asynctask.hpp"

class MaterialSaveTask : public IAsyncTask<std::string>
{
public:
    MaterialSaveTask(const URI& uri, const MaterialAsset& asset);

private:
    const MaterialAsset m_asset;

    const URI m_uri;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


