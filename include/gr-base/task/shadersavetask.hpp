#pragma once

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/asset/URI.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include <string>

class ShaderSaveTask : public IAsyncTask<std::string>
{
public:
    ShaderSaveTask(const URI& uri, const ShaderAsset& asset);

private:
    const ShaderAsset m_asset;

    const URI m_uri;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


