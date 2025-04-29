#include "gr-base/core/task/saveGameObjectTask.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/baserender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/console.hpp"

#include <filesystem>

SaveGameObjectTask::SaveGameObjectTask(const GameObject *go, std::string path) : m_gameObject(go), m_stream(nullptr)
{
    auto filename = go->getName() + ".gmdx";

    auto temp_file = std::filesystem::temp_directory_path() / filename;

    doInBackground(temp_file);

    if (std::filesystem::is_directory(path))
    {
        path += "/" + filename;
    }

    if (File::Exists(path))
    {
        File::Delete(path);
    }

    File::Copy(temp_file, path);
}

void SaveGameObjectTask::doInBackground(std::string path) {
    auto name = m_gameObject->getName();

    Console::LogInfo("[ SaveGameObjectTask ] Initialize task for import: %s, %s", name.c_str(), path.c_str());

    m_stream = std::make_shared<Stream>(path, std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'r', 'm', 'x'}, 1.0f, 0};
        if (!m_stream->writeS(&header, 1, false)) {
            return Console::LogError("[ SaveGameObjectTask ] Fail to write header.");
        }
    }

    readObjectSource(m_gameObject);

    int result = writeData();
    if (result != 0) {
        return Console::LogError("[ SaveGameObjectTask ] Fail to write metadata: %d", result);
    }
    Console::LogInfo("[ SaveGameObjectTask ] Modelsaved sucesfuly: %s", name.c_str());

    m_stream->close();
}

int SaveGameObjectTask::writeData()
{
    // write gameobject
    {
        if (writeGameObject(m_gameObject) != 0) {
            return 1;
        }

        DataSegment dataSegment = {HeaderType::OBJECT, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 2;
        }
        m_stream->flush();
    }

    // write material
    {
        if (writeMaterial() != 0) {
            return 5;
        }

        DataSegment dataSegment = {HeaderType::MATERIAL, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 6;
        }
        m_stream->flush();
    }

    // write script
    {
        if (writeScript() != 0) {
            return 7;
        }

        DataSegment dataSegment = {HeaderType::SCRIPT, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 8;
        }
        m_stream->flush();
    }

    // write texture
    {
        if (writeTexture() != 0) {
            return 3;
        }
        
        DataSegment dataSegment = {HeaderType::TEXTURE, static_cast<std::size_t>(m_stream->getDataSize())};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 4;
        }
        m_stream->flush(); 
    }

    // end file
    {
        DataSegment dataSegment = {HeaderType::NONE, 0};
        if (!m_stream->writeS(&dataSegment, 1, false)) {
            return 9;
        }

        m_stream->flush();
    }
    return 0;
}

int SaveGameObjectTask::writeGameObject(const GameObject *go)
{
    auto goname = go->getName();
    {
        auto gonsize = goname.size();
        if (!m_stream->write(&gonsize, sizeof(std::size_t)))
        {
            return 1;
        }

        if (!m_stream->write(goname.c_str(), gonsize * sizeof(char)))
        {
            return 2;
        }
    }

    auto gotag = go->getTag();
    {
        auto gotsize = gotag.size();

        if (!m_stream->write(&gotsize, sizeof(std::size_t)))
        {
            return 3;
        }
        if (!m_stream->write(gotag.c_str(),  gotsize * sizeof(char)))
        {
            return 4;
        }
    }


    auto active = go->getActive();
    {
        if (!m_stream->write(&active,  sizeof(bool)))
        {
            return 5;
        }

    }
    auto enable = go->getStatic();
    {
        if (!m_stream->write(&enable,  sizeof(bool)))
        {
            return 6;
        }
    }

    auto transform = go->getTransform();
    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!m_stream->write(&ccount, sizeof(std::size_t)))
    {
        return 7;
    }

    int result = 0;
    for (auto &child : children) {
        result = writeGameObject(child->getGameObject());
        if (result != 0)
        {
            break;
        }
    }
    return result;
}

int SaveGameObjectTask::writeTexture()
{
    auto textureCount = m_textureMap.size();
    if (!m_stream->write(&textureCount, sizeof(std::size_t)))
    {
        return 1;
    }

    for (auto [goname, mname, tpath] : m_textureMap)
    {
        // write game object name
        {
            auto gonsize = goname.size();
            if (!m_stream->write(&gonsize, sizeof(std::size_t)))
            {
                return 2;
            }

            if (!m_stream->write(goname.c_str(), gonsize * sizeof(char)))
            {
                return 3;
            }
        }

        // write material name
        {
            auto mnsize = mname.size();
            if (!m_stream->write(&mnsize, sizeof(std::size_t)))
            {
                return 4;
            }

            if (!m_stream->write(mname.c_str(), mnsize * sizeof(char)))
            {
                return 5;
            }
        }

        // write texture path
        {
            auto tpsize = tpath.size();
            if (!m_stream->write(&tpsize, sizeof(std::size_t)))
            {
                return 4;
            }

            if (!m_stream->write(tpath.c_str(), tpsize * sizeof(char)))
            {
                return 5;
            }
        }
    }
    return 0;
}

int SaveGameObjectTask::writeMaterial()
{
    auto materialCount = m_materialMap.size();
    if (!m_stream->write(&materialCount, sizeof(std::size_t)))
    {
        return 1;
    }

    for (auto [goname, mname, material] : m_materialMap)
    {
        // write game object name
        {
            auto gonsize = goname.size();
            if (!m_stream->write(&gonsize, sizeof(std::size_t)))
            {
                return 2;
            }

            if (!m_stream->write(goname.c_str(), gonsize * sizeof(char)))
            {
                return 3;
            }
        }

        // write material name
        {
            auto mnsize = mname.size();
            if (!m_stream->write(&mnsize, sizeof(std::size_t)))
            {
                return 4;
            }

            if (!m_stream->write(mname.c_str(), mnsize * sizeof(char)))
            {
                return 5;
            }
        }

        // deserialize material
        std::size_t bufferSize = 0;
        {
            Stream temp;
            material->serialize(&temp);
            bufferSize = temp.getDataSize();

            temp.close();
        }

        if (!m_stream->write(&bufferSize, sizeof(std::size_t))) {
            return 6;
        }

        material->serialize(m_stream.get());
    }
    return 0;
}

int SaveGameObjectTask::writeScript()
{
    auto scriptCount = m_scriptMap.size();
    if (!m_stream->write(&scriptCount, sizeof(std::size_t)))
    {
        return 1;
    }

    for (auto &&[goname, scriptname, script] : m_scriptMap)
    {
        // write game object name
        {
            auto gonsize = goname.size();
            if (!m_stream->write(&gonsize, sizeof(std::size_t)))
            {
                return 2;
            }

            if (!m_stream->write(goname.c_str(), gonsize * sizeof(char)))
            {
                return 3;
            }
        }

        // write script data
        {
            auto cnsize = scriptname.size();
            if (!m_stream->write(&cnsize, sizeof(std::size_t)))
            {
                return 4;
            }

            if (!m_stream->write(scriptname.c_str(), cnsize * sizeof(char)))
            {
                return 5;
            }

            auto pool = ComponentRegistry::Find(scriptname);

            Stream temp;
            pool->serialize(script, &temp);
            std::size_t data_size = temp.getDataSize();
            temp.close();

            if (!m_stream->write(&data_size, sizeof(std::size_t))) {
                return 6;
            }
            
            pool->serialize(script, m_stream.get());
        }
    }
    return 0;
}

void SaveGameObjectTask::readObjectSource(const GameObject *go)
{
    auto goname = go->getName();
    auto entity = go->getEntity();
    // copy script
    {
        for (auto &&[name, component] : entity->getComponentMap())
        {
            m_scriptMap.emplace_back(goname, name, component);
        }
    }

    // copy material
    {
        BaseRender *baseRender = nullptr;
        if (go->hasComponent<SkinnedMeshRender>())
        {
            baseRender = static_cast<BaseRender *>(go->getComponent<SkinnedMeshRender>());
        } else if (go->hasComponent<MeshRender>())
        {
            baseRender = static_cast<BaseRender *>(go->getComponent<MeshRender>());
        }

        if (baseRender != nullptr)
        {
            for (auto material : baseRender->getMaterial())
            {
                auto mname = AssetsManager::GetRefMaterialToName(material);
                if (mname.empty())
                {
                    continue;
                }

                for (auto texture : material->getTextures())
                {
                    if (texture == nullptr)
                    {
                        continue;
                    }

                    auto tpath = AssetsManager::GetRefTextureName(texture);
                    
                    m_textureMap.emplace_back(goname, mname, tpath);
                }
                m_materialMap.emplace_back(goname, mname, material);
            }
        }
    }
    auto transform = go->getTransform();
    
    for (auto child : transform->getChildren())
    {
        readObjectSource(child->getGameObject());
    }
}




