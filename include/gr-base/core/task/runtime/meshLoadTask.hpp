#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include <functional>
#include <string>

class MeshLoadTask : public IAsyncTask<MeshID>
{
public:
    MeshLoadTask(const std::string& file, std::function<void (MeshID)> func);

private:
    std::function<void (MeshID)> m_func;

    std::string m_file;

    MeshID doInBackground() override;

    void onPostExecute(MeshID result) override;
};
