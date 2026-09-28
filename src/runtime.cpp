#include "gr-base/runtime.hpp"

#include "gr-base/core/assetProvider.hpp"
#include "gr-base/core/assetResolver.hpp"
#include "gr-base/core/assetDB.hpp"

#include "gr-base/debug/debugDraw.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/renderer.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/eventManager.hpp"
#include "gr-base/managers/renderPipelineManager.hpp"
#include "gr-base/managers/sceneManager.hpp"
#include "gr-base/managers/taskmanager.hpp"
#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/systemRegistry.hpp"

#include "gr-base/file/file.hpp"

class EngineFileProvider : public IAssetProvider
{
public:
    EngineFileProvider()
    {
        AssetDB::Load(Scheme_Engine, gfile::get_data_directory() + "/asset.db");
    }

    ~EngineFileProvider()
    {
        AssetDB::Save(Scheme_Engine, gfile::get_data_directory() + "/asset.db");
    }

    std::string resolve(const std::string& path) override
    {
        return gfile::get_data_directory() + "/assets/" + path;
    }
};

void Runtime::initialize()
{
    MainThreadDispatcher::Initialize();

    EventManager::Initialize();

    AssetDB::Initialize();

    AssetResolver::Initialize();

    AssetResolver::Registry<EngineFileProvider>(Scheme_Engine);

    ComponentRegistry::Initialize();
    SystemRegistry::Initialize();

    Threadpool::Initialize();

    TaskManager::Initialize();

    AssetsManager::Initialize();

    RenderPipelineManager::Initialize();

    SceneManager::Initialize();

    Renderer::Initialize();

    // DebugDraw::Initialize();
}

void Runtime::shutdown()
{
    // DebugDraw::Shutdown();

    AssetResolver::Shutdown();

    AssetDB::Shutdown();

    Renderer::Shutdown();

    SceneManager::Shutdown();

    RenderPipelineManager::Shutdown();

    AssetsManager::Shutdown();

    TaskManager::Shutdown();

    Threadpool::Shutdown();

    ComponentRegistry::Shutdown();
    SystemRegistry::Shutdown();

    EventManager::Shutdown();

    MainThreadDispatcher::Shutdown();
}

void Runtime::update()
{
    MainThreadDispatcher::Dispatch();
}

