#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <memory>

class GameObjectSaveTask
{
public:
    GameObjectSaveTask(const GameObject *go, std::string path);

private:
    std::shared_ptr<Stream> m_stream;

    const GameObject *m_gameObject;

protected:
    std::vector<std::tuple<std::string, std::string, std::string>> m_textureMap;
    
    std::vector<std::tuple<std::string, std::string, Material *>> m_materialMap;
    
    std::vector<std::tuple<std::string, std::string, Component *>> m_scriptMap;

    void doInBackground(std::string path);

    int writeData();

    int writeGameObject(const GameObject *go);

    int writeTexture();

    int writeMaterial();

    int writeScript();

    void readObjectSource(const GameObject *go);
};




