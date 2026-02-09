#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/batchingmanager.hpp"

#include "gr-base/managers/rendermanager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include <utility>

MeshRender::MeshRender()
{}

MeshRender::~MeshRender()
{
    RenderManager::Remove(entityID);
}

void MeshRender::Init()
{
    RenderManager::Insert(entityID);
}

bool MeshRender::Serialize(Stream* stream) const
{
    return BaseRender::Serialize(stream);
}

bool MeshRender::Deserialize(Stream* stream)
{
    return BaseRender::Deserialize(stream);
}



