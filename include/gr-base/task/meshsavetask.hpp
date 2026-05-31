#pragma once

#include "gr-base/asset/meshasset.hpp"
#include "gr-base/core/thread/asynctask.hpp"
#include "gr-base/externals/hash.hpp"

class MeshSaveTask : public IAsyncTask<std::string>
{
public:
    MeshSaveTask(const std::string& path, const MeshAsset& asset);

private:
    const MeshAsset m_asset;

    const std::string m_path;

    void onPreExecute() override;

    std::string doInBackground() override;

    void onPostExecute(Result<std::string> result) override;
};


