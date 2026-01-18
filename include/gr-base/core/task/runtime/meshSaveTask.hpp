#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include <functional>
#include <string>

class MeshSaveTask : public IAsyncTask<MeshID>
{
public:
    MeshSaveTask(const std::string& file, MeshID id, std::function<void (MeshID)> func);

private:
    std::function<void (MeshID)> m_func;

    std::string m_file;

    MeshID m_meshID;

    MeshID doInBackground() override;

    void onPostExecute(MeshID result) override;
};

