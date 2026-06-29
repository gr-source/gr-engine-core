#pragma once

#include "gr-base/asset/meshasset.hpp"
#include "gr-base/asset/URI.hpp"

#include "gr-base/core/thread/asynctask.hpp"

class MeshSaveTask : public IAsyncTask<std::string>
{
public:
    MeshSaveTask(const URI& path, const MeshAsset& asset);

private:
    const MeshAsset m_asset;

    const URI m_uri;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


