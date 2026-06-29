#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/asset/textureAsset.hpp"
#include "gr-base/asset/URI.hpp"

class TextureBuildTask : public IAsyncTask<TextureAsset>
{
public:
    TextureBuildTask(const std::string& file, const URI& uri);

private:
    const std::string m_file;

    const URI m_uri;

    void onPreExecute() override;

    TextureAsset doInBackground() override;

    void onPostExecute(Result<TextureAsset> result) override;
};


