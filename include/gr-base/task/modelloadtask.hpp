#pragma once

#include "gr-base/asset/modelasset.hpp"
#include "gr-base/core/thread/asynctask.hpp"
#include "gr-base/ecs/default.hpp"

#include <functional>
#include <string>

class ModelLoadTask : public IAsyncTask<int>
{
public:
    template <typename ...Args>
    ModelLoadTask(const std::string& file, Args&&... args);

private:
    std::function<void (Handle)> m_func;

    std::string m_file;

    ModelAsset m_asset;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;

    Handle CreateFromAsset(const ModelAsset* asset, Handle parent = HANDLE_INVALID);
};

template <typename ...Args>
inline ModelLoadTask::ModelLoadTask(const std::string& file, Args&&... args) : m_file(file), m_func(nullptr)
{
    if constexpr (sizeof...(Args) > 0)
        m_func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
}


