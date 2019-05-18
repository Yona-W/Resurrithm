#include "MusicsManager.h"
#include "Music.h"
#include "SusAnalyzer.h"
#include "SettingManager.h"

using namespace std;
using namespace filesystem;

typedef lock_guard<mutex> LockGuard;


MusicsManager::MusicsManager()
	: loading(false)
	, analyzer(make_unique<SusAnalyzer>(192))
{}

MusicsManager::~MusicsManager()
{
	if (loadWorker.joinable()) loadWorker.join();
	for (auto p : categories) p->Release();
}

void MusicsManager::CreateMusicCache()
{
	{
		LockGuard lock(flagMutex);
		loading = true;
	}

	for (auto p : categories) p->Release();
	categories.clear();

	const auto mlpath = SettingManager::GetRootDirectory() / SU_MUSIC_DIR;
	for (const auto& fdata : directory_iterator(mlpath)) {
		if (!is_directory(fdata)) continue;

		auto category = CategoryParameter::Create(analyzer.get(), fdata);
		if (!category) continue;

		categories.push_back(category);
	}

	{
		LockGuard lock(flagMutex);
		loading = false;
	}
}

bool MusicsManager::Reload(const bool async)
{
	if (IsReloading()) return false;

	if (async) {
		// NOTE: detachでロジック的には問題ないはずだが、保険のためjoinで同期
		if (loadWorker.joinable()) loadWorker.join();

		thread loadthread([this] { CreateMusicCache(); });
		loadWorker.swap(loadthread);
	}
	else {
		CreateMusicCache();
	}

	return true;
}

bool MusicsManager::IsReloading() const
{
	LockGuard lock(flagMutex);
	return loading;
}

CategoryParameter* MusicsManager::GetCategory(uint32_t index) const
{
	if (index >= GetCategorySize()) return nullptr;

	auto ptr = categories[index];
	ptr->AddRef();

	return ptr;
}

void MusicsManager::RegisterType(asIScriptEngine* engine)
{
	engine->RegisterObjectType(SU_IF_MUSIC_MANAGER, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_MUSIC_MANAGER, "bool Reload(bool = false)", asMETHOD(MusicsManager, Reload), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MUSIC_MANAGER, "bool IsReloading()", asMETHOD(MusicsManager, IsReloading), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MUSIC_MANAGER, "uint GetCategorySize()", asMETHOD(MusicsManager, GetCategorySize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MUSIC_MANAGER, SU_IF_CATEGORY "@ GetCategory(uint)", asMETHOD(MusicsManager, GetCategory), asCALL_THISCALL);
}

