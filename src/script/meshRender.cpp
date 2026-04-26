#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/renderSystem.hpp"

MeshRender::MeshRender()
{}

MeshRender::~MeshRender()
{}

void MeshRender::Init()
{
    RenderSystem::Insert(entityID);
}

void MeshRender::Destroy()
{
    RenderSystem::Remove(entityID);
}

bool MeshRender::Serialize(Stream* stream) const
{
    return BaseRender::Serialize(stream);
}

bool MeshRender::Deserialize(Stream* stream)
{
    return BaseRender::Deserialize(stream);
}



