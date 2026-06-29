#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/asset/textureAsset.hpp"
#include "gr-base/asset/URI.hpp"

class TextureSaveTask : public IAsyncTask<std::string>
{
public:
    TextureSaveTask(const URI& uri, const TextureAsset& asset);

private:
    const TextureAsset m_asset;

    const URI m_uri;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


