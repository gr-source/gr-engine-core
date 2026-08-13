#include "gr-base/render/defaultPostProcess.hpp"
#include "gr-base/core/assetDB.hpp"
#include "gr-base/managers/assetsManager.hpp"

void DefaultPostProcess::Initialize()
{
    AssetsManager::LoadShader(AssetDB::FindByName("engine://shader/screen"));
}

void DefaultPostProcess::Execute()
{}
