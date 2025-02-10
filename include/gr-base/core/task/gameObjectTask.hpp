#pragma once

#include "taskStatus.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

using GameObjectFunc = std::function<void (GameObject *)>;

class GameObjectTask
{
public:
    GameObjectTask(const std::string &path, GameObjectFunc func);

private:
    std::unordered_map<std::string, GameObject *> m_tempGameObjectMap;
    
    std::vector<std::shared_ptr<GameObject>> m_tempGameObject;

    std::shared_ptr<Stream> m_stream;

    GameObjectFunc m_function;

    GameObject *m_rootNode;

    void upload(std::string path);

protected:
    void doInBackground(std::string path);

    int readData();

    int readGameObject(Transform *parent = nullptr);

    int readTexture();

    int readMaterial();

    int readScript();

};

