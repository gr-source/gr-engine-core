#include "gr-base/runtime.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/renderer.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/eventManager.hpp"
#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/ecs/type.hpp"

void Runtime::initialize()
{
    MainThreadDispatcher::Initialize();

    EventManager::Initialize();

    Type::Initialize();

    Threadpool::Initialize();

    TaskManager::Initialize();

    AssetsManager::Initialize();

    SceneManager::Initialize();

    Renderer::Initialize();

    DebugDraw::Initialize();
}

void Runtime::shutdown()
{
    DebugDraw::Shutdown();

    Renderer::Shutdown();

    SceneManager::Shutdown();

    AssetsManager::Shutdown();

    TaskManager::Shutdown();

    Threadpool::Shutdown();

    Type::Shutdown();

    EventManager::Shutdown();

    MainThreadDispatcher::Shutdown();
}

void Runtime::process()
{
    MainThreadDispatcher::Process();
}

