#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/renderSystem.hpp"

MeshRender::MeshRender()
{}

MeshRender::~MeshRender()
{}

void MeshRender::Init()
{
    RenderSystem::Insert(cacheID);
}

void MeshRender::Destroy()
{
    RenderSystem::Remove(cacheID);
}

bool MeshRender::Serialize(Stream* stream) const
{
    return BaseRender::Serialize(stream);
}

bool MeshRender::Deserialize(Stream* stream)
{
    return BaseRender::Deserialize(stream);
}



