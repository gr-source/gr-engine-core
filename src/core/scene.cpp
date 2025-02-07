#include "gr-base/core/scene.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/ecs/componentRegistry.hpp"

std::unordered_map<std::string, std::unique_ptr<Scene>> Scene::m_scenes;

std::hash<std::string> Scene::hash;

std::condition_variable Scene::cv;

std::mutex Scene::m_mutex;

Scene *Scene::m_current = nullptr;

bool Scene::bQuit = false;

void Scene::SetCurrent(Scene *scene) {
    m_current = scene;
}

Scene *Scene::GetCurrentScene() {
    return m_current;
}

bool Scene::HasScene() {
    return m_current != nullptr;
}

void Scene::Use(Scene *scene) {
    if (m_current && m_current != scene && m_current->m_enable) {
        m_current->setEnable(false);
    }
    
    if (scene && scene != m_current && !scene->m_enable) {
        scene->setEnable(true);
    }
    m_current = scene;
}

void Scene::Initialize() {
}

void Scene::Shutdown() {
    m_scenes.clear();

    m_current = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        bQuit = true;
    }
    cv.notify_all();
}

Scene *Scene::Create(const std::string &name) {
    auto &&[it, added] = m_scenes.emplace(name, std::make_unique<Scene>(name));

    Scene::Use(&(*it->second));

    // camera for game
    {
        auto camera = it->second->CreateGameObject("MainCamera");
        if (!camera) {
            m_scenes.erase(it);

            Scene::Use(nullptr);
            Console::LogError("[ Scene ] Falied to create scene, Camera do not created.");
            return nullptr;
        }

        camera->addComponent<Camera>();

        Scene::Init(camera);
    }

    return &(*it->second);
}

void Scene::LoadScene(const std::string &path, std::function<void(Scene *)> func) {
    Threadpool::AddTask(&Scene::Load, path, LoadFlags_Metadata | LoadFlags_Material | LoadFlags_Mesh | LoadFlags_Object | LoadFlags_Texture | LoadFlags_Script, func, nullptr);
}

/*
void UpdateComponent(Transform *transform) {
    transform->entity->reintantiate();

    for (auto child : transform->getChildren()) {
        UpdateComponent(child);
    }
}

void Scene::UpdateScript() {
    auto scene = GetCurrentScene();
    if (!scene) {
        return;
    }

    for (auto gameObject : scene->getObjectList()) {
        UpdateComponent(gameObject->getTransform());
    }
}
*/

void DestroyComponnet(Transform *transform) {
    transform->entity->destroy();

    for (auto child : transform->getChildren()) {
        DestroyComponnet(child);
    }
}

void Scene::DestroyScript() {
    auto scene = GetCurrentScene();
    if (!scene) {
        return;
    }

    for (auto gameObject : scene->getObjectList()) {
        DestroyComponnet(gameObject->getTransform());
    }
}

void Scene::SetupEssential() {
    auto scene = GetCurrentScene();
    if (!scene) { return; }
    
    
    auto pool = ComponentRegistry::Find("MeshRender");
    if (!pool) {
        return;
    }

    scene->for_each([pool] (GameObject *go) {
        if (go->hasComponent<MeshRender>()) {
            auto meshRender = go->getComponent<MeshRender>();

            pool->init(meshRender);

            meshRender->build();
        }
    });
}

void Scene::CheckRequestReinstanceScript() {
    auto script = ComponentRegistry::GetRequestScriptUpdate();
    if (script.empty()) { return; }

    for (auto &&[name, scene] : m_scenes) {
        for (auto name : script) {
            scene->reinstanceScript(name);
        }
    }
}

void Scene::CleanTemp() {
    /*
    auto gos = GameObject::m_temp;
    if (gos.empty()) { return; }

    for (auto &&[name, scene] : m_scenes) {
        for (auto go : gos) {
            scene->destroy(go);
        }
    }
    */
}

void Scene::Remove(GameObject *gameObject) {
    auto scene = GetCurrentScene();
    if (!scene) { return; }

    scene->remove(gameObject);
}

void Scene::Destoy(GameObject *gameObject) {
    auto scene = GetCurrentScene();
    if (!scene) { return; }
    
    scene->destroy(gameObject);
}

void Scene::Init(GameObject *gameObject) {
    auto entity = gameObject->getEntity();

    entity->init();

    if (entity->hasComponent<MeshRender>()) {
        entity->getComponent<MeshRender>()->build();
    }

    auto transform = entity->getComponent<Transform>();

    for (auto child : transform->getChildren()) {
        Scene::Init(child->getGameObject());
    }
}

Scene::Scene(const std::string &name) : m_name(name), m_enable(true), m_goCamera(nullptr), m_camera(nullptr) {
    m_goCamera = CreateEngineCamera();

    if (m_goCamera)
        m_camera = m_goCamera->getComponent<Camera>();
}

Scene::~Scene() {
    for (auto &gameObject : m_objects) {
        GameObject::Destroy(gameObject);
    }
}

std::string Scene::getName() const {
    return m_name;
}

void Scene::save(const std::string &path) {
    Stream stream(path, std::ios_base::out | std::ios_base::binary);

    // header data
    {
        FileHeader header{{'g', 'r', 's', 'c'}, 1.0f, static_cast<std::size_t>(stream.getDataSize())};
        std::strncpy(header.name, m_name.data(), 256);

        if (!stream.writeS(&header, 1, false)) {
            return Console::LogError("[ Scene ] Failed to write scene: 0");
        }
    }

    auto err = writeScene(&stream);
    if (err != 0) {
        return Console::LogError("[ SCENE ] Failed to write scene segment: %d", err);
    }

    m_componentCache.clear();

    DataSegment end{gHeaderType_None, 0};
    if (!stream.writeS(&end, 1, false)) {
        return Console::LogError("[ Scene ] Failed to write scene: 1");
    }

    Console::LogInfo("[ Scene ] Scene saved: %s", m_name.c_str());
}

GameObject *Scene::CreateGameObject(const std::string &name, const std::string &tag) {
    auto gameObject = GameObject::Create(name, tag);
    if (!gameObject) {
        return nullptr;
    }

    m_objects.push_back(gameObject);

    return gameObject;
}

std::vector<GameObject *> &Scene::getObjectList() {
    return m_objects;
}

void Scene::insert(GameObject *gameObject) {
    auto it = std::find(m_objects.begin(), m_objects.end(), gameObject);
    if (it != m_objects.end()) { return; }

    m_objects.push_back(gameObject);
}

void Scene::Clear() {
    m_objects.clear();
}

GameObject *Scene::getCameraObject() const {
    return m_goCamera;
}

Camera *Scene::getCamera() const {
    return m_camera;
}

std::unordered_map<std::string, std::unique_ptr<Scene>> &Scene::GetScenes() {
    return m_scenes;
}

void Scene::CleanCurrentScene() {
    auto scene = GetCurrentScene();
    if (!scene) { return; }

    auto temp_dir = File::GetTempFile("gr-engine/");
    if (!File::Exists(temp_dir)) {
        File::CreateDirectory(temp_dir);
    }

    scene->save(temp_dir + scene->getName() + ".scn.tmp");

    auto &objectList = scene->getObjectList();
    for (auto it = objectList.begin();it != objectList.end();) {
        GameObject::Destroy(*it);

        it = objectList.erase(it);
    }

    // GameObject::Clear();
}

void Scene::ReloadObjects(std::function<void(Scene *)> func) {
    auto scene = GetCurrentScene();
    if (!scene) {
        return;
    }

    auto temp_dir = File::GetTempFile("gr-engine/");
    if (!File::Exists(temp_dir)) {
        return;
    }

    auto &objectList = scene->getObjectList();
    for (auto it = objectList.begin();it != objectList.end();) {
        GameObject::Destroy(*it);

        it = objectList.erase(it);
    }

    // GameObject::Clear();
    
    Threadpool::AddTask(&Scene::Load, temp_dir + scene->getName() + ".scn.tmp", LoadFlags_Object | LoadFlags_Script, func, scene);
}

void Scene::InitializeObject() {
    auto scene = GetCurrentScene();
    if (!scene) { return; }

    for (auto go : scene->getObjectList()) {
        Scene::Init(go);
    }
}

// TODO: Object, Script because thread dead lock
void Scene::Load(const std::string &path, std::uint8_t flags, std::function<void(Scene *)> func, Scene *scene) {
    if (!File::Exists(path)) {
        Console::LogError("[ Scene ] File not found: " + path);
        return;
    }

    Stream stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        Console::LogError("[ Scene ] Faleid to open scene: " + path);
        return;
    }

    std::string name;

    // read header data
    {
        FileHeader header;
        if (!stream.readS(&header, 1)) {
            return Console::LogError("[ Scene ] Failed to read binary header: %s", path.c_str());
        }
        name = header.name;

        Console::LogInfo("[ Scene ] loading scene: %s - Version: %.2f", name.c_str(), header.version);
    }

    auto hasScene = scene != nullptr;

    scene = (hasScene ? scene : new Scene(name));

    std::unordered_map<std::string, GameObject *> m_tempObject;

    // read segment
    while (true) {
        DataSegment data;
        if (!stream.readS(&data, 1)) {
            break;
        }
        
        if (data.type == gHeaderType_None && data.sizeData == 0) {
            break;
        }
        
        auto nextPosition = stream.tellg() + static_cast<std::streampos>(data.sizeData);
        auto block_name = struct_name[data.type];

        std::cout << "Block: " << block_name << std::endl;

        if (data.sizeData <= 0) {
            stream.seekg(nextPosition, std::ios_base::beg);
            continue;
        }

        int result = -1;
        switch (data.type) {
        case gHeaderType_Metadata:
            if (flags & LoadFlags_Metadata) {
                readMetadata(&stream, scene);
            }
            break;
        case gHeaderType_Texture:
            if (flags & LoadFlags_Texture)
                result = readTexture(&stream);
            break;
        case gHeaderType_Material:
            if (flags & LoadFlags_Material)
                result = readMaterial(&stream);
            break;
        case gHeaderType_Mesh:
            if (flags & LoadFlags_Mesh)
                result = readMesh(&stream);
            break;
        case gHeaderType_Object:
            if (flags & LoadFlags_Object)
                result = readMainListGameObject(&stream, &(*scene), m_tempObject);
            break;
        case gHeaderType_Script:
            if (flags & LoadFlags_Script)
                result = readScript(&stream, m_tempObject);
            break;
        default:
            break;
        }

        if (result != 0 && result != -1) {
            Console::LogError("[ Scene ] Falied to read segment data: " + std::string(block_name) + " Code: " + std::to_string(result));
        }
        
        if (stream.tellg() != nextPosition) {
            Console::LogWarning("[ Scene ] Data segment: %s is not fully loaded.", block_name);
        }
        stream.seekg(nextPosition, std::ios_base::beg);
    }

    Event::Add(&Scene::SetupScene, scene);
    
    if (func)
        Event::Add(func, scene);
}

GameObject *Scene::CreateEngineCamera() {
    auto go = GameObject::Create("MainEngineCamera", "Engine");
    if (!go) {
        return nullptr;
    }

    auto camera = go->addComponent<Camera>();
    if (!camera) {
        return nullptr;
    }

    Scene::Init(go);
    return go;
}

void Scene::setEnable(bool value) {
    for (auto &gameObject : m_objects) {
        setDisableEntity(gameObject->getTransform(), value);
    }
    m_enable = value;
}

void Scene::setDisableEntity(Transform *transform, bool value) {
    transform->entity->setEnable(value);

    for (auto &child : transform->getChildren()) {
        setDisableEntity(child, value);
    }
}

int Scene::writeScene(Stream *stream) {
    DataSegment segment = {gHeaderType_None, 0};

    {
        if (writeMetadata(stream) != 0) {
            return 1;
        }
    
        segment = {gHeaderType_Metadata, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 2;
        }
    
        stream->flush();
    }

    // textures
    {
        if (writeTexture(stream) != 0) {
            return 3;
        }
    
        segment = {gHeaderType_Texture, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 4;
        }
    
        stream->flush();
    }

    // material
    {
        if (writeMaterial(stream) != 0) {
            return 5;
        }
    
        segment = {gHeaderType_Material, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 6;
        }
    
        stream->flush();
    }

    // Mesh
    {
        // if (writeMesh(streaming) != 0) {
        //     return 7;
        // }

        // segment = {gHeaderType_Mesh, static_cast<std::size_t>(streaming->getDataSize())};
        // if (!streaming->writeS(&segment, 1, false)) {
        //     return 8;
        // }

        // streaming->flush();
    }
    
    // Objects
    {
        if (writeObject(stream) != 0) {
            return 9;
        }
        
        segment = {gHeaderType_Object, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 10;
        }

        stream->flush();
    }

    // script
    {
        if (writeScript(stream) != 0) {
            return 11;
        }
        
        segment = {gHeaderType_Script, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 12;
        }

        stream->flush();
    }
    return 0;
}

int Scene::writeMetadata(Stream *stream) {
    auto snsize = m_name.size();
    if (!stream->write(&snsize, sizeof(std::size_t))) {
        return 1;
    }

    if (!stream->write(m_name.c_str(), snsize * sizeof(char))) {
        return 2;
    }

    auto transform = m_goCamera->getTransform();
    transform->Serialize(stream);
    return 0;
}

int Scene::writeTexture(Stream *stream) {
    auto &textureMap = AssetsManager::GetTextureMap();

    auto tcount = AssetsManager::GetTextureCount();
    if (!stream->write(&tcount, sizeof(std::size_t))) {
        return 2;
    }

    for (auto &&[path, ptr] : textureMap) {
        std::size_t psize = path.size();
        if (!stream->write(&psize, sizeof(std::size_t))) {
            return 3;
        }

        if (!stream->write(path.c_str(), psize * sizeof(char))) {
            return 4;
        }
    }

    return 0;
}

int Scene::writeMaterial(Stream *stream) {
    auto &materialMap = AssetsManager::GetMaterialMap();

    auto mcount = materialMap.size();
    if (!stream->write(&mcount, sizeof(std::size_t))) {
        return 1;
    }

    for (auto &&[name, material] : materialMap) {
        std::size_t nsize = name.size();
        if (!stream->write(&nsize, sizeof(std::size_t))) {
            return 2;
        }

        if (!stream->write(name.c_str(), nsize * sizeof(char))) {
            return 3;
        }

        material->serialize(stream);
    }

    return 0;
}

int Scene::writeMesh(Stream *streaming) {
    auto meshCount = AssetsManager::GetMeshCount();

    if (!streaming->writeS(&meshCount, 1)) {
        return 1;
    }

    for (auto &&[name, mesh] : AssetsManager::GetMeshListMap()) {
        auto size = name.size();

        if (!streaming->writeS(&size, 1)) {
            return 2;
        }

        if (!streaming->writeS(name.data(), size)) {
            return 3;
        }

        auto result = mesh->serialize(streaming);
        if (result != 0) {
            return 4 + result;
        }
    }
    return 0;
}

int Scene::writeObject(Stream *stream) {
    auto objectCount = m_objects.size();

    if (!stream->write(&objectCount, sizeof(std::size_t))) {
        return 1;
    }

    std::function<void(GameObject *)> func = std::bind(&Scene::writeGameObject, this, stream, std::placeholders::_1);

    if (func)
        std::for_each(m_objects.begin(), m_objects.end(), func);
    return 0;
}

int Scene::writeGameObject(Stream *stream, GameObject *gameObject) {
    // write name
    {
        auto goname = gameObject->getName();
        auto gonsize = goname.size();

        if (!stream->write(&gonsize, sizeof(std::size_t))) {
            return 1;
        }

        if (!stream->write(goname.c_str(), gonsize * sizeof(char))) {
            return 2;
        }
    }

    // write tag
    {
        auto gotag = gameObject->getTag();
        auto gotsize = gotag.size();

        if (!stream->write(&gotsize, sizeof(std::size_t))) {
            return 3;
        }
        
        if (!stream->write(gotag.data(), gotsize * sizeof(char))) {
            return 4;
        }
    }

    // write status object
    {
        auto active = gameObject->getActive();
        if (!stream->write(&active,  sizeof(bool))) {
            return 5;
        }

        auto enable = gameObject->getEnable();
        if (!stream->write(&enable, sizeof(bool))) {
            return 6;
        }
    }

    // Write Component
    auto entity = gameObject->getEntity();

    std::list<std::tuple<std::string, Component *>> m_list;
    for (auto [name, ptr] : entity->getComponentMap()) {
        m_list.emplace_front(name, &(*ptr));
    }

    for (auto &&[name, ptr] : m_list) {
        m_componentCache.emplace_back(gameObject->getName(), name, ptr);
    }

    auto transform = gameObject->getTransform();
    auto children = transform->getChildren();

    auto ccount = children.size();
    if (!stream->write(&ccount, sizeof(std::size_t))) {
        return 7;
    }

    int result = 0;
    for (auto &child : children) {
        result = writeGameObject(stream, child->getGameObject());
        if (result != 0) {
            break;
        }
    }
    
    return result;
}

int Scene::writeScript(Stream *streaming) {
    auto size = m_componentCache.size();

    streaming->writeS(&size, 1);

    for (auto &&[objectName, componentName, ptr] : m_componentCache) {
        
        // ObjectName
        size = objectName.size();

        streaming->writeS(&size, 1);
        streaming->writeS(objectName.data(), size);

        // ComponentName
        size = componentName.size();

        streaming->writeS(&size, 1);
        streaming->writeS(componentName.data(), size);

        Stream temp;
        ptr->Serialize(&temp);
        std::size_t data_size = temp.getDataSize();

        streaming->writeS(&data_size, 1);
        ptr->Serialize(streaming);

        temp.close();
    }
    m_componentCache.clear();
    return 0;
}

int Scene::readMetadata(Stream *stream, Scene *scene) {
    std::size_t snsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&snsize), sizeof(std::size_t))) {
        return 1;
    }

    std::string name(snsize, '\0');
    if (!stream->read(name.data(), snsize * sizeof(char))) {
        return 2;
    }
    scene->m_name = name;

    auto transform = scene->m_goCamera->getTransform();
    transform->Deserialize(stream);
    return 0;
}

int Scene::readTexture(Stream *stream) {
    std::size_t tcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&tcount), sizeof(std::size_t))) {
        return 1;
    }

    std::vector<std::string> pathList(tcount);
    for (auto &path : pathList) {
        std::size_t psize = 0;
        if (!stream->read(reinterpret_cast<char *>(&psize), sizeof(std::size_t))) {
            return 2;
        }

        path.resize(psize, '\0');
        if (!stream->read(path.data(), psize * sizeof(char))) {
            return 3;
        }
    }
    
    for (auto &path : pathList) {
        // AssetsManager::LoadTexture(path);
    }
    return 0;
}

int Scene::readMaterial(Stream *stream) {
    std::size_t mcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t))) {
        return 1;
    }

    for (std::size_t i=0;i<mcount;i++) {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
            return 2;
        }

        std::string name(nsize, '\0');
        if (!stream->read(name.data(), nsize * sizeof(char))) {
            return 3;
        }

        auto material = AssetsManager::CreateMaterial(name);
        material->deserialize(stream);
    }
    return 0;
}

// TODO: pode gerarr erro, por favor corrija
int Scene::readMesh(Stream *streaming) {
    grm::size meshCount = 0;
    if (!streaming->readS(&meshCount, 1)) {
        return 1;
    }

    for (auto i=0;i<meshCount;i++) {
        std::size_t size = 0;
        if (!streaming->readS(&size, 1)) {
            return 2;
        }

        std::string name(size, '\0');
        if (!streaming->readS(name.data(), size)) {
            return 3;
        }

        auto mesh = AssetsManager::CreateMesh(name, name);
        
        auto result = mesh->deserialize(streaming);
        if (result != 0) {
            return 4 + result;
        }
    }
    return 0;
}

int Scene::readMainListGameObject(Stream *stream, Scene *scene, std::unordered_map<std::string, GameObject *> &tempObject) {
    std::size_t gocount = 0;
    if (!stream->read(reinterpret_cast<char *>(&gocount), sizeof(std::size_t))) {
        return 1;
    }

    int result = 0;
    for (std::size_t i=0;i<gocount;i++) {
        result = readGameObject(stream, scene, nullptr, tempObject);
        if (result != 0) {
            break;
        }
    }

    return result;
}

void CrateGameObject(Transform *&outTransform, GameObject *&outObject, std::condition_variable &cv, std::mutex &mutex, Scene *scene, Transform *parent, bool active, bool enable, std::string name, std::string tag, int &result) {
    GameObject *gameObject = nullptr;
    {
        gameObject = parent == nullptr ? scene->CreateGameObject(name, tag) : GameObject::Create(name, tag);
        if (!gameObject) {
            std::lock_guard<std::mutex> lock(mutex);
            result = 1;
            cv.notify_one();
            return;
        }

        if (gameObject->hasComponent<Transform>())
            gameObject->destroy<Transform>();

        gameObject->setTransform(gameObject->addComponent<Transform>());
    }
    
    gameObject->setActive(active);
    gameObject->setEnable(enable);

    auto transform = gameObject->getTransform();
    transform->setParent(parent);

    {
        std::lock_guard<std::mutex> lock(mutex);
        outTransform = transform;
        outObject = gameObject;

        result = 0;
    }

    cv.notify_one();
}

int Scene::readGameObject(Stream *stream, Scene *scene, Transform *parent, std::unordered_map<std::string, GameObject *> &tempObject) {
    std::size_t gonsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&gonsize), sizeof(std::size_t))) {
        return 1;
    }

    std::string goname(gonsize, '\0');
    if (!stream->read(goname.data(), gonsize * sizeof(char))) {
        return 2;
    }

    std::size_t gotsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&gotsize), sizeof(std::size_t))) {
        return 3;
    }

    std::string gotag(gotsize, '\0');
    if (!stream->read(gotag.data(), gotsize * sizeof(char))) {
        return 4;
    }

    bool active = true;
    if (!stream->read(reinterpret_cast<char *>(&active), sizeof(bool))) {
        return 5;
    }

    bool enable = true;
    if (!stream->read(reinterpret_cast<char *>(&enable), sizeof(bool))) {
        return 6;
    }

    GameObject *gameObject = nullptr;
    Transform *transform = nullptr;

    int presult = -1;

    Event::Add(CrateGameObject, std::ref(transform), std::ref(gameObject), std::ref(cv), std::ref(m_mutex), std::ref(scene), parent, active, enable, goname, gotag, std::ref(presult));

    std::unique_lock<std::mutex> lock(m_mutex);
    cv.wait(lock, [&transform, &presult] {
        return transform || bQuit || presult >= 0;
    });

    if (gameObject)
        tempObject.emplace(goname, gameObject);
        
    if (presult != 0) {
        return 2;
    }
    
    if (bQuit) {
        return 1;
    }
    lock.unlock();

    std::size_t ccount = 0;
    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 8;
    }

    int result = 0;
    for (std::size_t i=0;i<ccount;i++) {
        result = readGameObject(stream, scene, transform, tempObject);
        if (result != 0) {
            break;
        }
    }

    transform = nullptr;
    gameObject = nullptr;

    return result;
}

int Scene::readScript(Stream *streaming, std::unordered_map<std::string, GameObject *> &tempObject) {
    std::size_t nsize = 0;
    std::size_t size = 0;
    int result = 0;

    result = streaming->readS(&size, 1) ? 0 : 1;
    if (result != 0) {
        return result;
    }

    for (int i=0;i<size;i++) {
        // ObjectName
        result = streaming->readS(&nsize, 1) ? 0 : 2;
        if (result != 0) {
            break;
        }

        std::string objectName(nsize, '\0');
        result = streaming->readS(objectName.data(), nsize) ? 0 : 3;
        if (result != 0) {
            break;
        }

        // ComponentName
        result = streaming->readS(&nsize, 1) ? 0 : 4;
        if (result != 0) {
            break;
        }
        
        std::string componentName(nsize, '\0');
        result = streaming->readS(componentName.data(), nsize) ? 0 : 5;
        if (result != 0) {
            break;
        }

        nsize = 0;

        std::size_t data_size = 0;
        result = streaming->readS(&data_size, 1) ? 0 : 6;
        if (result != 0) {
            break;
        }

        auto position = streaming->tellg() + static_cast<std::streampos>(data_size);
    
        Component *component = nullptr;
        int result = -1;

        auto it = tempObject.find(objectName);
        if (it == tempObject.end()) {
            streaming->seekg(position, std::ios_base::beg);
            continue;
        }
        Event::Add(&Scene::SetupScript, it->second, componentName, std::ref(cv), std::ref(component), std::ref(result));

        std::unique_lock<std::mutex> lock(m_mutex);
        cv.wait(lock, [&component, &result] {
            return component || bQuit || result >= 0;
        });
        if (bQuit) {
            break;
        }
        lock.unlock();
        
        if (data_size <= 0) {
            continue;
        }

        if (result != 0) {
            Console::LogError("Falied to load object: %s, %s", objectName.c_str(), componentName.c_str());
            
            streaming->seekg(position, std::ios_base::beg);
            continue;
        }
        result = -1;

        Threadpool::AddTask(&Scene::ProcessScript, component, streaming->getPath(), streaming->tellg());

        // jump to end script buffer
        streaming->seekg(position, std::ios_base::beg);
    }
    return result;
}

void Scene::SetupScene(Scene *scene) {
    auto it = m_scenes.find(scene->getName());
    if (it == m_scenes.end())
        m_scenes.emplace(scene->getName(), std::unique_ptr<Scene>(scene));

    for (auto &go : scene->getObjectList()) {
        Initialize(go);
    }
}

void Scene::SetupScript(GameObject *gameObject, std::string name, std::condition_variable &cv, Component *&component, int &result) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        result = 0;
    }

    auto entt = gameObject->getEntity();
    if (!entt) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            result = 2;
        }
        cv.notify_one();
        return;
    }

    try {
        std::lock_guard<std::mutex> lock(m_mutex);
        component = entt->hasComponent(name) ? entt->getComponent(name) : entt->addComponent(name);
    } catch (const std::exception &e) {
        Console::LogError("[ Scene ] %s", e.what());
        std::lock_guard<std::mutex> lock(m_mutex);
        result = 3;
    }
    cv.notify_one();
}

void Scene::ProcessScript(Component *component, std::string path, std::streampos position) {
    Stream stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        Console::LogError("[ Scene ] Falied to load binary: ", path.c_str());
        return;
    }

    stream.seekg(position, std::ios_base::beg);

    try {
        component->Deserialize(&stream);
    } catch (const std::exception &e) {
        Console::LogError("[ Scene ] %s", e.what());
    }
}

static std::vector<std::string> names = {"Light", "MeshRender", "MeshFilter"};
void Scene::Initialize(GameObject *go) {
    auto entity = go->getEntity();
    
    for (auto cname : names) {
        if (entity->hasComponent(cname)) {
            entity->init();

            if (cname == "MeshRender") {
                static_cast<MeshRender *>(entity->getComponent(cname))->build();
            }
        }
    }
    
    for (auto child : go->getTransform()->getChildren()) {
        Scene::Initialize(child->getGameObject());
    }
}

bool Scene::remove(GameObject *gameObject) {
    auto it = std::remove(m_objects.begin(), m_objects.end(), gameObject);
    if (it == m_objects.end()) { return false; }

    m_objects.erase(it, m_objects.end());
    return true;
}

bool Scene::destroy(GameObject *gameObject) {
    if (!remove(gameObject)) {
        Console::LogWarning("[ Scene ] GameObject don't has in scene.");
    }

    GameObject::Destroy(gameObject);
    return true;
}

void Scene::for_each(std::function<void(GameObject *)> func) {
    for (auto object : m_objects) {
        for_each(object->getTransform(), func);
    }
}

void Scene::for_each(Transform *transform, std::function<void(GameObject *)> func) {
    func(transform->getGameObject());

    for (auto child : transform->getChildren()) {
        for_each(child, func);
    }
}

void Scene::reinstanceScript(const std::string &name) {
    Scene::for_each([name] (GameObject *go) {
        auto entt = go->getEntity();

        if (!entt->hasComponent(name)) { return; }

        entt->destroy(name);

        entt->addComponent(name);
    });
}

