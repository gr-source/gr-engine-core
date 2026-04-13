#include "gr-base/task/savemodeltask.hpp"
#include "gr-base/core/gameObject.hpp"

SaveModelTask::SaveModelTask(const std::string& file, GameObjectHandle handle) : m_handle(handle)
{}

void SaveModelTask::onPreExecute()
{
    GameObject* gameObject = GameObject::Get(m_handle);

    std::string name = gameObject->GetName();
    bool isStatic = gameObject->GetIsStatic();
}

// private
int SaveModelTask::doInBackground()
{
    return 0;
}

void SaveModelTask::onPostExecute(int result)
{}



