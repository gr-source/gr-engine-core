#include "gr-base/core/task/gameObjectTask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

GameObjectTask::GameObjectTask(const std::string &path, GameObjectFunc func) : m_function(func), m_stream(nullptr)
{
    Threadpool::AddTask(&GameObjectTask::doInBackground, this, path);
}

void GameObjectTask::upload(std::string path)
{
    auto scene = Scene::GetCurrentScene();
    if (scene != nullptr) {
        Scene::Use(nullptr);
    }

    auto go = GameObject::Instantiate(m_rootNode);
    // if (go->hasComponent<MeshRender>())
    
    Scene::Use(scene);
    
    Console::LogInfo("[ GameObjectTask ] Model loaded sucessfuly %s.", go->getName().c_str());

    if (m_function) {
        m_function(go);
    }
    AssetsManager::RemoveTask(path);
}

void GameObjectTask::doInBackground(std::string path)
{
    m_stream = std::make_unique<Stream>(path, std::ios_base::in | std::ios_base::binary);
    if (!m_stream->is_open())
    {
        Console::LogError("[ GameObjectTask ] Falied to open file: %s", path.c_str());

        return AssetsManager::RemoveTask(path);
    }

    Console::LogInfo("[ GameObjectTask ] Loading model: %s", path.c_str());

    FileHeader header;
    if (!m_stream->read(reinterpret_cast<char *>(&header), sizeof(FileHeader))) {
        Console::LogError("[ GameObjectTask ] Falied to read header binary.");
        return AssetsManager::RemoveTask(path);
    }
    
    if (header.signature[0] != 'g' && header.signature[1] != 'r' && header.signature[2] != 'm' && header.signature[3] != 'x') {
        Console::LogError("[ GameObjectTask ] Invalid binary type.");
        return AssetsManager::RemoveTask(path);
    }

    auto result = readData();
    if (result != 0) {
        Console::LogError("[ GameObjectTask ] Fail to read data %d", result);
        return AssetsManager::RemoveTask(path);
    }

    Event::Add(&GameObjectTask::upload, this, path);
}

int GameObjectTask::readData() {
    while (true) {
        DataSegment segment;
        if (!m_stream->read(reinterpret_cast<char *>(&segment), sizeof(DataSegment)))
        {
            return 1;
        }

        if (segment.type == gHeaderType_None && segment.sizeData == 0)
        {
            break;
        }

        auto segment_name = struct_name[segment.type];

        std::cout << "Segment: " << segment_name << std::endl;

        auto next_position = m_stream->tellg() + static_cast<std::streampos>(segment.sizeData);

        int result = -1;
        switch (segment.type) {
        case gHeaderType_Object:
            result = readGameObject();
            break;
        case gHeaderType_Texture:
            result = readTexture();
            break;
        case gHeaderType_Material:
            result = readMaterial();
            break;
        case gHeaderType_Script:
            result = readScript();
            break;
        default:
            break;
        }

        if (result != 0) {
            Console::LogError("[ AssetsManager ] Falied to load segment: %s code: %d", struct_name[segment.type], result);
        }

        if (m_stream->tellg() != next_position) {
            m_stream->seekg(next_position, std::ios_base::beg);

            Console::LogWarning("[ AssetsManager ] Block position is invalid");
        }
    }

    return 0;
}

int GameObjectTask::readGameObject(Transform *parent)
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
    gameObject->setEnable(bEnable);

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

int GameObjectTask::readTexture()
{
    std::size_t nsize = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
    {
        return 1;
    }

    std::string goname(nsize, '\0');
    if (!m_stream->read(goname.data(), nsize * sizeof(char)))
    {
        return 2;
    }

    std::size_t tcount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&tcount), sizeof(std::size_t)))
    {
        return 3;
    }

    for (std::size_t i={};i<tcount;i++)
    {
        std::size_t psize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&psize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string path(psize, '\0');
        if (!m_stream->read(path.data(), psize * sizeof(char)))
        {
            return 5;
        }

        AssetsManager::LoadTexture(path);
    }

    std::size_t ccount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 6;
    }

    int result = 0;
    for (std::size_t i=0;i<ccount;i++) {
        result = readTexture();
        if (result != 0) {
            break;
        }
    }
    return result;
}

int GameObjectTask::readMaterial()
{
    std::size_t sname = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&sname), sizeof(std::size_t)))
    {
        return 1;
    }

    std::string name(sname, '\0');
    if (!m_stream->read(name.data(), sname * sizeof(char)))
    {
        return 2;
    }

    // if (gameObjectMap.find(name) == gameObjectMap.end()) {
    //     std::cout << name << std::endl;
    //     return 3;
    // }

    // auto go = gameObjectMap[name];

    std::size_t mcount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t)))
    {
        return 4;
    }

    for (std::size_t i={};i<mcount;i++)
    {
        std::size_t mnsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&mnsize), sizeof(std::size_t)))
        {
            return 5;
        }

        std::string mname(mnsize, '\0');
        if (!m_stream->read(mname.data(), mnsize * sizeof(char)))
        {
            return 6;
        }

        std::size_t bufferSize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&bufferSize), sizeof(std::size_t)))
        {
            return 7;
        }

        auto next = m_stream->tellg() + static_cast<std::streampos>(bufferSize);
        
        if (AssetsManager::FindMaterial(mname) == nullptr)
        {
            auto material = AssetsManager::CreateMaterial(mname);
            material->deserialize(m_stream.get());
        }

        if (m_stream->tellg() != next)
        {
            m_stream->seekg(next, std::ios_base::beg);
        }
    }

    std::size_t ccount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t)))
    {
        return 8;
    }

    int result = 0;
    for (std::size_t i=0;i<ccount;i++)
    {
        result = readMaterial();
        if (result != 0)
        {
            break;
        }
    }

    return result;
}

int GameObjectTask::readScript()
{
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

    std::size_t ccount = 0;
    if (!m_stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t)))
    {
        return 3;
    }


    for (std::size_t i={};i<ccount;i++)
    {
        std::size_t nsize = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return 4;
        }

        std::string cname(nsize, '\0');
        if (!m_stream->read(cname.data(), nsize * sizeof(char)))
        {
            return 5;
        }

        std::size_t stream_size = 0;
        if (!m_stream->read(reinterpret_cast<char *>(&stream_size), sizeof(std::size_t)))
        {
            return 6;
        }
        
        auto next = m_stream->tellg() + static_cast<std::streampos>(stream_size);

        auto it = m_tempGameObjectMap.find(goname);
        if (it == m_tempGameObjectMap.end())
        {
            m_stream->seekg(next, std::ios_base::beg);
            continue;
        }

        if (stream_size <= 0)
            continue;

        auto entity = it->second->getEntity();

        auto component = entity->hasComponent(cname) ? entity->getComponent(cname) : entity->addComponent(cname);
        if (component != nullptr)
        {
            component->Deserialize(m_stream.get());
        }

        if (m_stream->tellg() != next)
        {
            Console::LogWarning("[ SceneTask ] %s script %s read data imcompleted!", goname.c_str(), cname.c_str());
            
            m_stream->seekg(next, std::ios_base::beg);
            continue;
        }
    }


    if (!m_stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 7;
    }

    int result =0;
    for (std::size_t i=0;i<ccount;i++) {
        result = readScript();
        if (result != 0) {
            break;
        }
    }

    return result;
}







