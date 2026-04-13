#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/ecs/default.hpp"

class SaveModelTask : public IAsyncTask<int>
{
public:
    SaveModelTask(const std::string& file, GameObjectHandle handle);

private:
    GameObjectHandle m_handle;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;
};


