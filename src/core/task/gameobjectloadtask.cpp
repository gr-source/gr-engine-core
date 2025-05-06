#include "gr-base/core/task/gameobjectloadtask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/binary/gHeader.hpp"

GameObjectLoadTask::GameObjectLoadTask(const std::string &path, GameObjectFunc func) : m_function(func), m_stream(nullptr)
{
    Threadpool::AddTask(&GameObjectLoadTask::doInBackground, this, path);
}

void GameObjectLoadTask::upload(std::string path)
{
    if (m_rootNode == nullptr)
    {
        return Console::LogError("[ GameObjectLoadTask ] Fail object loaded is invalid.");
    }

    auto scene = Scene::GetCurrentScene();
    if (scene != nullptr)
    {
        Scene::Use(nullptr);
    }

    auto go = GameObject::Instantiate(m_rootNode);
    
    Scene::Use(scene);
    
    Console::LogInfo("[ GameObjectLoadTask ] Model loaded sucessfuly %s.", go->getName().c_str());

    if (m_function) {
        m_function(go);
    }
    AssetsManager::RemoveTask(path);
}

void GameObjectLoadTask::doInBackground(std::string path)
{
    m_stream = std::make_unique<Stream>(path, std::ios_base::in | std::ios_base::binary);
    if (!m_stream->is_open())
    {
        Console::LogError("[ GameObjectLoadTask ] Falied to open file: %s", path.c_str());

        return AssetsManager::RemoveTask(path);
    }

    Console::LogInfo("[ GameObjectLoadTask ] Loading model: %s", path.c_str());

    FileHeader header;
    if (!m_stream->read(reinterpret_cast<char *>(&header), sizeof(FileHeader)))
    {
        Console::LogError("[ GameObjectLoadTask ] Falied to read header binary.");
        
        return AssetsManager::RemoveTask(path);
    }
    
    if (std::strcmp(header.signature, "grmx") != 0) {
        Console::LogError("[ GameObjectLoadTask ] Invalid binary type.");
        return AssetsManager::RemoveTask(path);
    }

    auto result = readData();
    if (result != 0)
    {
        Console::LogError("[ GameObjectLoadTask ] Fail to read data %d", result);
        
        return AssetsManager::RemoveTask(path);
    }

    Event::Add(&GameObjectLoadTask::upload, this, path);
}

int GameObjectLoadTask::readData() {
    while (true) {
        DataSegment segment;
        if (!m_stream->read(reinterpret_cast<char *>(&segment), sizeof(DataSegment)))
        {
            return 1;
        }

        if (segment.type == HeaderType::NONE && segment.sizeData == 0)
        {
            break;
        }

        auto segment_name = HeaderName[segment.type];

        auto next_position = m_stream->tellg() + static_cast<std::streampos>(segment.sizeData);
        if (segment.sizeData <= 0)
        {
            m_stream->seekg(next_position, std::ios_base::beg);
            continue;
        }

        int result = -1;
        switch (segment.type)
        {
            case HeaderType::OBJECT:
                result = readGameObject();
                break;
            case HeaderType::SCRIPT:
                result = readScript();
                break;
            case HeaderType::MATERIAL:
                result = readMaterial();
                break;
            case HeaderType::TEXTURE:
                result = readTexture();
                break;
            default:
                break;
        }

        if (result != 0) {
            Console::LogError("[ AssetsManager ] Falied to load segment: %s code: %d", HeaderName[segment.type], result);
        }

        if (m_stream->tellg() != next_position) {
            m_stream->seekg(next_position, std::ios_base::beg);

            Console::LogWarning("[ AssetsManager ] Block position is invalid");
        }
    }

    return 0;
}

int GameObjectLoadTask::readGameObject(Transform *parent)
{
    // read name
    std::size_t gonsize = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
    {
        return 1;
    }

    std::string goname(gonsize, '\0');
    if (!m_stream->read(goname.data(), gonsize * sizeof(char)))
    {
        return 2;
    }

    // read tag
    std::size_t gotsize = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&gotsize), sizeof(std::size_t))) {
        return 3;
    }

    std::string gotag(gotsize, '\0');
    if (!m_stream->read(gotag.data(), gotsize * sizeof(char))) {
        return 4;
    }

    bool bActive = false;
    bool bEnable = false;

    // read status
    {
        if (!m_stream->read(reinterpret_cast<char *>(&bActive), sizeof(bool))) {
            return 5;
        }

        if (!m_stream->read(reinterpret_cast<char *>(&bEnable), sizeof(bool))) {
            return 6;
        }
    }

    // create

    auto gameObject = &(*m_tempGameObject.emplace_back(std::make_shared<GameObject>(goname, gotag)));
    gameObject->setActive(bActive);
    gameObject->setStatic(bEnable);

    m_tempGameObjectMap.emplace(goname, gameObject);
    
    auto transform = gameObject->getTransform();
    if (parent != nullptr) {
        transform->setParent(parent);
    } else {
        m_rootNode = gameObject;
    }

    std::size_t ccount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 7;
    }
    
    int result = 0;
    for (int i=0;i<ccount;i++) {
        result = readGameObject(transform);
        if (result != 0) {
            break;
        }
    }
    transform = nullptr;

    return result;
}

int GameObjectLoadTask::readTexture()
{
    std::size_t textureCount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&textureCount), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i=0;i<textureCount;i++)
    {
        // read game object name
        std::size_t gonsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(gonsize, '\0');
        if (!m_stream->read(goname.data(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // read material name
        std::size_t mnsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&mnsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string mname(mnsize, '\0');
        if (!m_stream->read(mname.data(), mnsize * sizeof(char)))
        {
            return 5;
        }

        // read texture path
        std::size_t tpsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&tpsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string tpath(tpsize, '\0');
        if (!m_stream->read(tpath.data(), tpsize * sizeof(char)))
        {
            return 5;
        }

        // std::cout << "SetMaterial: " << goname << " " << mname << " " << tpath << std::endl;
    }
    return 0;
}

int GameObjectLoadTask::readMaterial()
{
    std::size_t sname = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&sname), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<sname;i++)
    {
        // read game object name
        std::size_t gonsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(gonsize, '\0');
        if (!m_stream->read(goname.data(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // read material name
        std::size_t mnsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&mnsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string mname(mnsize, '\0');
        if (!m_stream->read(mname.data(), mnsize * sizeof(char)))
        {
            return 5;
        }

        // deserialize material
        std::size_t bufferSize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&bufferSize), sizeof(std::size_t))) {
            return 6;
        }

        auto next = m_stream->tellg() + static_cast<std::streampos>(bufferSize);

        auto material = AssetsManager::FindMaterial(mname);
        if (material == nullptr)
        {
            material = AssetsManager::CreateMaterial(mname);
            material->deserialize(m_stream.get());
        }

        auto it = m_tempGameObjectMap.find(goname);
        if (it != m_tempGameObjectMap.end())
        {
            auto go = it->second;
            if (go->hasComponent<MeshRender>())
            {
                auto meshRender = go->getComponent<MeshRender>();
                if (!meshRender->hasMaterial(material))
                    meshRender->addMaterial(material);
            } else if (go->hasComponent<SkinnedMeshRender>())
            {
                auto meshRender = go->getComponent<SkinnedMeshRender>();
                if (!meshRender->hasMaterial(material))
                    meshRender->addMaterial(material);
            }

        }

        if (m_stream->tellg() != next)
        {
            m_stream->seekg(next, std::ios_base::beg);
        }
    }
    return 0;
}

int GameObjectLoadTask::readScript()
{
    std::size_t gonsize = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
    {
        return 1;
    }

    for (std::size_t i={};i<gonsize;i++)
    {
        std::size_t gonsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t)))
        {
            return 2;
        }

        std::string goname(gonsize, '\0');
        if (!m_stream->read(goname.data(), gonsize * sizeof(char)))
        {
            return 3;
        }

        // read material name
        std::size_t cnsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&cnsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string scriptname(cnsize, '\0');
        if (!m_stream->read(scriptname.data(), cnsize * sizeof(char)))
        {
            return 5;
        }

        std::size_t data_size = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&data_size), sizeof(std::size_t))) {
            return 6;
        }

        auto next = m_stream->tellg() + static_cast<std::streampos>(data_size);

        auto it = m_tempGameObjectMap.find(goname);
        if (it == m_tempGameObjectMap.end())
        {
            m_stream->seekg(next, std::ios_base::beg);
            continue;
        }
        
        auto entity = it->second->getEntity();

        auto component = entity->hasComponent(scriptname) ? entity->getComponent(scriptname) : entity->addComponent(scriptname);
        if (component != nullptr && data_size > 0)
        {
            component->Deserialize(m_stream.get());
        }

        if (m_stream->tellg() != next)
        {
            Console::LogWarning("[ SceneTask ] %s script %s read data imcompleted!", goname.c_str(), scriptname.c_str());
            
            m_stream->seekg(next, std::ios_base::beg);
            continue;
        }
    }

    return 0;
}







