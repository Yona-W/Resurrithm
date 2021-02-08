﻿#include "ExecutionManager.h"

using namespace std;

void ExecutionManager::Fire(const string & message)
{
    for (auto &scene : scenes) scene->OnEvent(message);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ExecutionManager::WriteLog(const string &message) const
{
    
    spdlog::info(message);
}

ScenePlayer *ExecutionManager::CreatePlayer()
{
    auto player = new ScenePlayer(this);
    player->AddRef();

    BOOST_ASSERT(player->GetRefCount() == 1);
    return player;
}

SSoundMixer *ExecutionManager::GetDefaultMixer(const string &name) const
{
    if (name == "BGM") {
        mixerBgm->AddRef();
        return mixerBgm;
    }
    if (name == "SE") {
        mixerSe->AddRef();
        return mixerSe;
    }
    return nullptr;
}

SSettingItem *ExecutionManager::GetSettingItem(const string &group, const string &key) const
{
    const auto si = settingManager->GetSettingItem(group, key);
    if (!si) return nullptr;
    auto result = new SSettingItem(si);
    result->AddRef();
    return result;
}

void ExecutionManager::GetStoredResult(DrawableResult *result) const
{
    *result = lastResult;
}
