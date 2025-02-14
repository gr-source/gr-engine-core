#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <memory>
#include <string>

class SaveSceneTask
{
public:

    SaveSceneTask(const Scene *scene);

private:
    const Scene *m_scene;

    std::shared_ptr<Stream> m_stream;

protected:
    std::vector<std::tuple<std::string, std::string, Component *, BaseChunk *>> m_scriptCache;

    void doInBackground(std::string path);

    int writeData();

    int writeMetadata();

    int writeTexture();

    int writeMaterial();

    int writeMesh();

    int writeObject();

    int writeScript();

    int writeGameObject(const GameObject *go);
};


