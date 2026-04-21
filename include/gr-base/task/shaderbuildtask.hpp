#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include <string>
#include <tuple>

class ShaderBuildTask : public IAsyncTask<int>
{
public:
    ShaderBuildTask(const std::string& file, const std::string& source);

private:
    std::string m_file;

    std::string m_source;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;

    std::tuple<std::string, std::string> ReadingSource();
};

