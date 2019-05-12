#include "ExecutionManager.h"
#include "Scene.h"
#include "ScenePlayer.h"
#include "SettingManager.h"

using namespace std;

void ExecutionManager::Fire(const string& message)
{
	for (auto& scene : scenes) scene->OnEvent(message);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ExecutionManager::WriteLog(const string & message) const
{
	auto log = spdlog::get("main");
	log->info(message);
}

ScenePlayer* ExecutionManager::CreatePlayer()
{
	auto player = new ScenePlayer(this);
	player->AddRef();

	SU_ASSERT(player->GetRefCount() == 1);
	return player;
}

SSettingItem* ExecutionManager::GetSettingItem(const string & group, const string & key) const
{
	const auto si = settingManager->GetSettingItem(group, key);
	if (!si) return nullptr;
	auto result = new SSettingItem(si);
	result->AddRef();
	return result;
}

void ExecutionManager::GetStoredResult(DrawableResult * result) const
{
	*result = *lastResult;
}
