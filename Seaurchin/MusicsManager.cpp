#include "MusicsManager.h"
#include "SusAnalyzer.h"
#include "SettingManager.h"

using namespace std;
using namespace filesystem;

typedef lock_guard<mutex> LockGuard;

// CategoryInfo ---------------------------

CategoryInfo::CategoryInfo(const path& cpath)
{
	categoryPath = cpath;
	name = ConvertUnicodeToUTF8(categoryPath.filename());
}

CategoryInfo::~CategoryInfo()
= default;

// ReSharper disable once CppMemberFunctionMayBeStatic
void CategoryInfo::Reload(bool recreateCache) const
{}


// MusicsManager ---------------------------

MusicsManager::MusicsManager()
	: loading(false)
	, analyzer(make_unique<SusAnalyzer>(192))
	, categoryIndex(0)
	, musicIndex(-1)
	, variantIndex(-1)
	, state(MusicSelectionState::Category)
{}

MusicsManager::~MusicsManager()
{
	if (loadWorker.joinable()) loadWorker.join();
}

void MusicsManager::CreateMusicCache()
{
	{
		LockGuard lock(flagMutex);
		loading = true;
	}

	categories.clear();

	const auto mlpath = SettingManager::GetRootDirectory() / SU_MUSIC_DIR;
	for (const auto& fdata : directory_iterator(mlpath)) {
		if (!is_directory(fdata)) continue;

		auto category = make_shared<CategoryInfo>(fdata);
		for (const auto& mdir : directory_iterator(fdata)) {
			if (!is_directory(mdir)) continue;
			for (const auto& file : directory_iterator(mdir)) {
				if (is_directory(file)) continue;
				if (file.path().extension() != ".sus") continue;     //これ大文字どうすんの
				analyzer->Reset();
				analyzer->LoadFromFile(file.path(), true);
				auto music = find_if(category->Musics.begin(), category->Musics.end(), [&](const shared_ptr<MusicMetaInfo> info) {
					return info->SongId == analyzer->SharedMetaData.USongId;
					});
				if (music == category->Musics.end()) {
					music = category->Musics.insert(category->Musics.begin(), make_shared<MusicMetaInfo>());
					(*music)->SongId = analyzer->SharedMetaData.USongId;
					(*music)->Name = analyzer->SharedMetaData.UTitle;
					(*music)->Artist = analyzer->SharedMetaData.UArtist;
					(*music)->JacketPath = mdir.path().filename() / ConvertUTF8ToUnicode(analyzer->SharedMetaData.UJacketFileName);
				}
				auto score = make_shared<MusicScoreInfo>();
				score->Path = mdir.path().filename() / file.path().filename();
				score->BackgroundPath = ConvertUTF8ToUnicode(analyzer->SharedMetaData.UBackgroundFileName);
				score->WavePath = ConvertUTF8ToUnicode(analyzer->SharedMetaData.UWaveFileName);
				score->Designer = analyzer->SharedMetaData.UDesigner;
				score->BpmToShow = analyzer->SharedMetaData.ShowBpm;
				score->Difficulty = analyzer->SharedMetaData.DifficultyType;
				score->DifficultyName = analyzer->SharedMetaData.UExtraDifficulty;
				score->Level = analyzer->SharedMetaData.Level;
				(*music)->Scores.push_back(score);
			}
		}
		categories.push_back(category);
	}

	{
		LockGuard lock(flagMutex);
		loading = false;
	}
}

MusicSelectionState MusicsManager::ReloadMusic(const bool async)
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	if (async) {
		if (loadWorker.joinable()) {
			spdlog::get("main")->error(u8"MusicCursor::ReloadMusicは実行中です。");
			return MusicSelectionState::Reloading;
		}

		thread loadthread([this] { CreateMusicCache(); });
		loadWorker.swap(loadthread);
	}
	else {
		CreateMusicCache();
	}

	return MusicSelectionState::Success;
}

bool MusicsManager::IsReloading() const
{
	LockGuard lock(flagMutex);
	return loading;
}

MusicSelectionState MusicsManager::ResetState()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	categoryIndex = 0;
	musicIndex = -1;
	variantIndex = -1;
	state = MusicSelectionState::Category;

	return MusicSelectionState::Success;
}

shared_ptr<CategoryInfo> MusicsManager::GetCategoryAt(const int32_t relative) const
{
	// NOTE: リロード中は負数が返ってくる
	const auto categorySize = GetCategorySize();
	if (categorySize <= 0) return nullptr;

	auto actual = relative + categoryIndex;
	while (actual < 0) actual += categorySize;
	return categories[actual % categorySize];
}

shared_ptr<MusicMetaInfo> MusicsManager::GetMusicAt(const int32_t relative) const
{
	// NOTE: リロード中はnullが返ってくる
	const auto category = GetCategoryAt(0);
	if (!category) return nullptr;

	const auto musicSize = category->Musics.size();
	if (musicSize == 0) return nullptr;

	auto actual = relative + musicIndex;
	while (actual < 0) actual += musicSize;
	return category->Musics[actual % musicSize];
}

shared_ptr<MusicScoreInfo> MusicsManager::GetScoreVariantAt(const int32_t relative) const
{
	// NOTE: リロード中はnullが返ってくる
	const auto music = GetMusicAt(relative);
	if (!music) return nullptr;

	const auto size = music->Scores.size();
	if (size == 0 || variantIndex < 0) return nullptr;

	return music->Scores[min(SU_TO_UINT32(variantIndex), size - 1)];
}

path MusicsManager::GetSelectedScorePath() const
{
	const auto cat = GetCategoryAt(0);
	const auto score = GetScoreVariantAt(0);
	if (!cat || !score) return path();

	return SettingManager::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(cat->GetName()) / score->Path;
}

string MusicsManager::GetSelectedScorePathString() const
{
	return ConvertUnicodeToUTF8(GetSelectedScorePath());
}

string MusicsManager::GetPrimaryString(const int32_t relativeIndex) const
{
	switch (state) {
	case MusicSelectionState::Category:
		return GetCategoryName(relativeIndex);
	case MusicSelectionState::Music:
		return GetMusicName(relativeIndex);
	default:
		return u8"Unavailable!";
	}
}

string MusicsManager::GetCategoryName(const int32_t relativeIndex) const
{
	const auto category = GetCategoryAt(relativeIndex);
	return category ? category->GetName() : u8"Unavailable!";
}

string MusicsManager::GetMusicName(const int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	return music ? music->Name : u8"Unavailable!";
}

string MusicsManager::GetArtistName(const int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	return music ? music->Artist : u8"Unavailable!";
}

string MusicsManager::GetMusicJacketFileName(const int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	if (!music || music->JacketPath.empty()) return u8"";

	const auto result = SettingManager::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(GetCategoryName(0)) / music->JacketPath;
	return ConvertUnicodeToUTF8(result);
}

string MusicsManager::GetBackgroundFileName(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	if (!variant || variant->BackgroundPath.empty()) return "";

	const auto result = SettingManager::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(GetCategoryName(0)) / variant->Path.parent_path() / variant->BackgroundPath;
	return ConvertUnicodeToUTF8(result);
}

int MusicsManager::GetDifficulty(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->Difficulty : 0;
}

int MusicsManager::GetLevel(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->Level : 0;
}

double MusicsManager::GetBpm(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->BpmToShow : 0;
}

string MusicsManager::GetExtraLevel(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->DifficultyName : u8"";
}

string MusicsManager::GetDesignerName(const int32_t relativeIndex) const
{
	const auto variant = GetScoreVariantAt(relativeIndex);
	return variant ? variant->Designer : u8"";
}

MusicSelectionState MusicsManager::Enter()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category:
		if (GetCategorySize() <= 0) return MusicSelectionState::Error;
		state = MusicSelectionState::Music;
		musicIndex = 0;
		variantIndex = 0;
		return MusicSelectionState::Success;
	case MusicSelectionState::Music:
		//選曲終了
		state = MusicSelectionState::OutOfFunction;
		return MusicSelectionState::Confirmed;
	default:
		return MusicSelectionState::OutOfFunction;
	}
}

MusicSelectionState MusicsManager::Exit()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category:
		state = MusicSelectionState::OutOfFunction;
		break;
	case MusicSelectionState::Music:
		state = MusicSelectionState::Category;
		musicIndex = -1;
		variantIndex = -1;
		break;
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

MusicSelectionState MusicsManager::Next()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category: {
		const auto categorySize = GetCategorySize();
		if (categorySize <= 0 || categoryIndex < 0) return MusicSelectionState::Error;

		categoryIndex = (categoryIndex + 1) % categorySize;
		break;
	}
	case MusicSelectionState::Music: {
		const auto musicSize = GetMusicSize(0);
		const auto lastMusicIndex = musicIndex;
		if (musicSize <= 0 || musicIndex < 0) return MusicSelectionState::Error;

		musicIndex = (musicIndex + 1) % musicSize;
		const auto nextVariant = min(SU_TO_INT32(variantIndex), GetVariantSize(0) - 1);
		if (nextVariant < 0) {
			musicIndex = lastMusicIndex;
			return MusicSelectionState::Error;
		}
		variantIndex = SU_TO_UINT16(nextVariant);
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

MusicSelectionState MusicsManager::Previous()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Category: {
		const auto categorySize = GetCategorySize();
		if (categorySize <= 0 || categoryIndex < 0) return MusicSelectionState::Error;

		categoryIndex = (categoryIndex + categorySize - 1) % categorySize;
		break;

	}
	case MusicSelectionState::Music: {
		const auto musicSize = GetMusicSize(0);
		const auto lastMusicIndex = musicIndex;
		if (musicSize <= 0 || musicIndex < 0) return MusicSelectionState::Error;

		musicIndex = (musicIndex + musicSize - 1) % musicSize;
		const auto nextVariant = min(SU_TO_INT32(variantIndex), GetVariantSize(0) - 1);
		if (nextVariant < 0) {
			musicIndex = lastMusicIndex;
			return MusicSelectionState::Error;
		}
		variantIndex = SU_TO_UINT16(nextVariant);
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

MusicSelectionState MusicsManager::NextVariant()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Music: {
		const auto variantSize = GetVariantSize(0);
		if (variantSize <= 0 || variantIndex < 0) return MusicSelectionState::Error;

		variantIndex = (variantIndex + 1) % variantSize;
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

MusicSelectionState MusicsManager::PreviousVariant()
{
	if (IsReloading()) return MusicSelectionState::Reloading;

	switch (state) {
	case MusicSelectionState::Music: {
		const auto variantSize = GetVariantSize(0);
		if (variantSize <= 0 || variantIndex < 0) return MusicSelectionState::Error;

		variantIndex = (variantIndex + variantSize - 1) % variantSize;
		break;
	}
	default:
		return MusicSelectionState::OutOfFunction;
	}
	return MusicSelectionState::Success;
}

MusicSelectionState MusicsManager::GetState() const
{
	return IsReloading() ? MusicSelectionState::Reloading : state;
}

int32_t MusicsManager::GetCategorySize() const
{
	return IsReloading() ? -1 : SU_TO_INT32(categories.size());
}

int32_t MusicsManager::GetMusicSize(int32_t relativeIndex) const
{
	const auto category = GetCategoryAt(relativeIndex);
	return category ? SU_TO_INT32(category->Musics.size()) : -1;
}

int32_t MusicsManager::GetVariantSize(int32_t relativeIndex) const
{
	const auto music = GetMusicAt(relativeIndex);
	return music ? SU_TO_INT32(music->Scores.size()) : -1;
}

void MusicsManager::RegisterType(asIScriptEngine* engine)
{
	engine->RegisterEnum(SU_IF_MSCSTATE);
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "OutOfFunction", int(MusicSelectionState::OutOfFunction));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Category", int(MusicSelectionState::Category));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Music", int(MusicSelectionState::Music));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Confirmed", int(MusicSelectionState::Confirmed));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Reloading", int(MusicSelectionState::Reloading));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Error", int(MusicSelectionState::Error));
	engine->RegisterEnumValue(SU_IF_MSCSTATE, "Success", int(MusicSelectionState::Success));

	engine->RegisterObjectType(SU_IF_MSCURSOR, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " ReloadMusic(bool = false)", asMETHOD(MusicsManager, ReloadMusic), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " ResetState()", asMETHOD(MusicsManager, ResetState), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetSelectedScorePath()", asMETHOD(MusicsManager, GetSelectedScorePathString), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetPrimaryString(int = 0)", asMETHOD(MusicsManager, GetPrimaryString), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetCategoryName(int = 0)", asMETHOD(MusicsManager, GetCategoryName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetMusicName(int = 0)", asMETHOD(MusicsManager, GetMusicName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetArtistName(int = 0)", asMETHOD(MusicsManager, GetArtistName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetMusicJacketFileName(int = 0)", asMETHOD(MusicsManager, GetMusicJacketFileName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetBackgroundFileName(int = 0)", asMETHOD(MusicsManager, GetBackgroundFileName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetDifficulty(int = 0)", asMETHOD(MusicsManager, GetDifficulty), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetLevel(int = 0)", asMETHOD(MusicsManager, GetLevel), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "double GetBpm(int = 0)", asMETHOD(MusicsManager, GetBpm), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetExtraLevel(int = 0)", asMETHOD(MusicsManager, GetExtraLevel), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetDesignerName(int = 0)", asMETHOD(MusicsManager, GetDesignerName), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Next()", asMETHOD(MusicsManager, Next), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Previous()", asMETHOD(MusicsManager, Previous), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " NextVariant()", asMETHOD(MusicsManager, NextVariant), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " PreviousVariant()", asMETHOD(MusicsManager, PreviousVariant), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Enter()", asMETHOD(MusicsManager, Enter), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Exit()", asMETHOD(MusicsManager, Exit), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " GetState()", asMETHOD(MusicsManager, GetState), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetCategorySize()", asMETHOD(MusicsManager, GetCategorySize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetMusicSize(int = 0)", asMETHOD(MusicsManager, GetMusicSize), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetVariantSize(int = 0)", asMETHOD(MusicsManager, GetVariantSize), asCALL_THISCALL);
}

