#include "ExecutionManager.h"
#include "Scene.h"
#include "ScenePlayer.h"
#include "SettingManager.h"

using namespace std;

void ExecutionManager::Fire(const string& message)
{
	for (auto& scene : scenes) scene->OnEvent(message);
}

void ExecutionManager::WriteLog(ScriptLogSeverity severity, const string & message)
{
	auto log = spdlog::get("main");
	switch (severity) {
	case ScriptLogSeverity::Trace:
		log->trace(message);
		break;
	case ScriptLogSeverity::Debug:
		log->debug(message);
		break;
	case ScriptLogSeverity::Info:
		log->info(message);
		break;
	case ScriptLogSeverity::Warning:
		log->warn(message);
		break;
	case ScriptLogSeverity::Error:
		log->error(message);
		break;
	case ScriptLogSeverity::Critical:
		log->critical(message);
		break;
	}
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

