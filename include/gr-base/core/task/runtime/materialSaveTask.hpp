#pragma once


#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include <string>

class MaterialSaveTask : public IAsyncTask<int>
{
public:
    MaterialSaveTask(const std::string& file, const Material* material);

private:
    std::string m_file;

    const Material* m_material;

    int doInBackground() override;

    void onPostExecute(int result) override;
};

