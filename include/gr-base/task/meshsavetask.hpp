#pragma once

#include "gr-base/asset/meshasset.hpp"
#include "gr-base/core/thread/asynctask.hpp"

class MeshSaveTask : public IAsyncTask<int>
{
public:
    MeshSaveTask(const std::string& file, const MeshAsset& asset);

private:
    MeshAsset m_asset;

    std::string m_file;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;
};


