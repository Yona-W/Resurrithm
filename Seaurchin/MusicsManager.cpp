#include "MusicsManager.h"
#include "ExecutionManager.h"
#include "Config.h"
#include "Misc.h"

using namespace std;
using namespace filesystem;

typedef std::lock_guard<std::mutex> LockGuard;

//path sepath = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR;

MusicsManager::MusicsManager(ExecutionManager *exm)
{
    manager = exm;
    analyzer = make_unique<SusAnalyzer>(192);
}

MusicsManager::~MusicsManager()
= default;

void MusicsManager::Initialize()
{}

void MusicsManager::Reload(const bool async)
{
    if (IsReloading()) return;

    if (async) {
        thread loadthread([this] { CreateMusicCache(); });
        loadthread.detach();
    } else {
        CreateMusicCache();
    }
}

bool MusicsManager::IsReloading()
{
    LockGuard lock(flagMutex);
    return loading;
}

path MusicsManager::GetSelectedScorePath()
{
    const auto ci = manager->GetData<int>("Selected:Category");
    const auto mi = manager->GetData<int>("Selected:Music");
    const auto vi = manager->GetData<int>("Selected:Variant");
    auto result = Setting::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(categories[ci]->GetName());
    result /= categories[ci]->Musics[mi]->Scores[vi]->Path;
    return result;
}

void MusicsManager::CreateMusicCache()
{
    {
        LockGuard lock(flagMutex);
        loading = true;
    }

    categories.clear();

    const auto mlpath = Setting::GetRootDirectory() / SU_MUSIC_DIR;
    for (const auto& fdata : directory_iterator(mlpath)) {
        if (!is_directory(fdata)) continue;

        auto category = make_shared<CategoryInfo>(fdata);
        for (const auto& mdir : directory_iterator(fdata)) {
            if (!is_directory(mdir)) continue;
            for (const auto& file : directory_iterator(mdir)) {
                if (is_directory(file)) continue;
                if (file.path().extension() != ".sus") continue;     //これ大文字どうすんの
                analyzer->Reset();
                analyzer->LoadFromFile(file.path().wstring(), true);
                auto music = find_if(category->Musics.begin(), category->Musics.end(), [&](const std::shared_ptr<MusicMetaInfo> info) {
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

MusicSelectionCursor *MusicsManager::CreateMusicSelectionCursor()
{
    auto result = new MusicSelectionCursor(this);
    result->AddRef();

    BOOST_ASSERT(result->GetRefCount() == 1);
    return result;
}

// CategoryInfo ---------------------------

CategoryInfo::CategoryInfo(const path& cpath)
{
    categoryPath = cpath;
    name = ConvertUnicodeToUTF8(categoryPath.filename().wstring());
}

CategoryInfo::~CategoryInfo()
= default;

// ReSharper disable once CppMemberFunctionMayBeStatic
void CategoryInfo::Reload(bool recreateCache) const
{

}

//MusicSelectionCursor ------------------------------------------

std::shared_ptr<CategoryInfo> MusicSelectionCursor::GetCategoryAt(const int32_t relative) const
{
    const auto categorySize = GetCategorySize();
    if (categorySize <= 0) return nullptr;
    auto actual = relative + categoryIndex;
    while (actual < 0) actual += categorySize;
    return manager->categories[actual % categorySize];
}

std::shared_ptr<MusicMetaInfo> MusicSelectionCursor::GetMusicAt(const int32_t relative) const
{
    const auto musicSize = GetMusicSize(0);
    if (musicSize <= 0) return nullptr;
    auto actual = relative + musicIndex;
    while (actual < 0) actual += musicSize;
    return GetCategoryAt(0)->Musics[actual % musicSize];
}

std::shared_ptr<MusicScoreInfo> MusicSelectionCursor::GetScoreVariantAt(const int32_t relative) const
{
    const auto music = GetMusicAt(relative);
    if (!music) return nullptr;
    const auto variant = min(int32_t(variantIndex), GetVariantSize(relative) - 1);
    if (variant < 0) return nullptr;
    return music->Scores[variant];
}

MusicSelectionCursor::MusicSelectionCursor(MusicsManager *manager)
    : manager(manager)
    , categoryIndex(0)
    , musicIndex(-1)
    , variantIndex(-1)
    , state(MusicSelectionState::Category)
{}

MusicSelectionState MusicSelectionCursor::ReloadMusic(const bool async)
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    manager->Reload(async);

    return MusicSelectionState::Success;
}

MusicSelectionState MusicSelectionCursor::ResetState()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    categoryIndex   = 0;
    musicIndex      = -1;
    variantIndex    = -1;
    state           = MusicSelectionState::Category;

    return MusicSelectionState::Success;
}

std::string MusicSelectionCursor::GetPrimaryString(const int32_t relativeIndex) const
{
    switch (state) {
        case MusicSelectionState::Category:
            return GetCategoryName(relativeIndex);
        case MusicSelectionState::Music:
            return GetMusicName(relativeIndex);
        default:
            return "";
    }
}

string MusicSelectionCursor::GetCategoryName(const int32_t relativeIndex) const
{
    const auto category = GetCategoryAt(relativeIndex);
    return category ? category->GetName() : "Unavailable!";
}

string MusicSelectionCursor::GetMusicName(const int32_t relativeIndex) const
{
    const auto music = GetMusicAt(relativeIndex);
    return music ? music->Name : "Unavailable!";
}

string MusicSelectionCursor::GetArtistName(const int32_t relativeIndex) const
{
    const auto music = GetMusicAt(relativeIndex);
    return music ? music->Artist : "Unavailable!";
}

string MusicSelectionCursor::GetMusicJacketFileName(const int32_t relativeIndex) const
{
    const auto music = GetMusicAt(relativeIndex);
    if (!music || music->JacketPath.empty()) return "";
    const auto result = (Setting::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(GetCategoryName(0)) / music->JacketPath).wstring();
    return ConvertUnicodeToUTF8(result);
}

string MusicSelectionCursor::GetBackgroundFileName(const int32_t relativeIndex) const
{
    const auto variant = GetScoreVariantAt(relativeIndex);
    if (!variant || variant->BackgroundPath.empty()) return "";
    auto result = Setting::GetRootDirectory() / SU_MUSIC_DIR / ConvertUTF8ToUnicode(GetCategoryName(0)) / variant->Path.parent_path() / variant->BackgroundPath;
    return ConvertUnicodeToUTF8(result.wstring());
}

int MusicSelectionCursor::GetDifficulty(const int32_t relativeIndex) const
{
    const auto variant = GetScoreVariantAt(relativeIndex);
    return variant ? variant->Difficulty : 0;
}

int MusicSelectionCursor::GetLevel(const int32_t relativeIndex) const
{
    const auto variant = GetScoreVariantAt(relativeIndex);
    return variant ? variant->Level : 0;
}

double MusicSelectionCursor::GetBpm(const int32_t relativeIndex) const
{
    const auto variant = GetScoreVariantAt(relativeIndex);
    return variant ? variant->BpmToShow : 0;
}

std::string MusicSelectionCursor::GetExtraLevel(const int32_t relativeIndex) const
{
    const auto variant = GetScoreVariantAt(relativeIndex);
    return variant ? variant->DifficultyName : "";
}

std::string MusicSelectionCursor::GetDesignerName(const int32_t relativeIndex) const
{
    const auto variant = GetScoreVariantAt(relativeIndex);
    return variant ? variant->Designer : "";
}

MusicSelectionState MusicSelectionCursor::Enter()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    switch (state) {
        case MusicSelectionState::Category:
            if (GetCategorySize() <= 0) return MusicSelectionState::Error;
            state = MusicSelectionState::Music;
            musicIndex = 0;
            variantIndex = 0;
            return MusicSelectionState::Success;
        case MusicSelectionState::Music:
            //選曲終了
            manager->manager->SetData<int>("Selected:Category", categoryIndex);
            manager->manager->SetData<int>("Selected:Music", musicIndex);
            manager->manager->SetData<int>("Selected:Variant", variantIndex);
            manager->manager->SetData("Player:Jacket", GetMusicJacketFileName(0));
            manager->manager->SetData("Player:Background", GetBackgroundFileName(0));
            state = MusicSelectionState::OutOfFunction;
            return MusicSelectionState::Confirmed;
        default:
            return MusicSelectionState::OutOfFunction;
    }
}

MusicSelectionState MusicSelectionCursor::Exit()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    switch (state) {
        case MusicSelectionState::Category:
            state = MusicSelectionState::OutOfFunction;
            break;
        case MusicSelectionState::Music:
            state = MusicSelectionState::Category;
            break;
        default:
            return MusicSelectionState::OutOfFunction;
    }
    return MusicSelectionState::Success;
}

MusicSelectionState MusicSelectionCursor::Next()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    switch (state) {
        case MusicSelectionState::Category: {
            const auto categorySize = GetCategorySize();
            if (categorySize <= 0) return MusicSelectionState::Error;
            categoryIndex = (categoryIndex + 1) % categorySize;
            break;
        }
        case MusicSelectionState::Music: {
            const auto musicSize = GetMusicSize(0);
            const auto lastMusicIndex = musicIndex;
            if (musicSize <= 0) return MusicSelectionState::Error;
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

MusicSelectionState MusicSelectionCursor::Previous()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    switch (state) {
        case MusicSelectionState::Category: {
            const auto categorySize = GetCategorySize();
            if (categorySize <= 0) return MusicSelectionState::Error;
            categoryIndex = (categoryIndex + categorySize - 1) % categorySize;
            break;

        }
        case MusicSelectionState::Music: {
            const auto musicSize = GetMusicSize(0);
            const auto lastMusicIndex = musicIndex;
            if (musicSize <= 0) return MusicSelectionState::Error;
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

MusicSelectionState MusicSelectionCursor::NextVariant()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    switch (state) {
        case MusicSelectionState::Music: {
            const auto variantSize = GetVariantSize(0);
            if (variantSize <= 0) return MusicSelectionState::Error;
            variantIndex = (variantIndex + 1) % variantSize;
            break;
        }
        default:
            return MusicSelectionState::OutOfFunction;
    }
    return MusicSelectionState::Success;
}

MusicSelectionState MusicSelectionCursor::PreviousVariant()
{
    if (manager->IsReloading()) return MusicSelectionState::Reloading;

    switch (state) {
        case MusicSelectionState::Music: {
            const auto variantSize = GetVariantSize(0);
            if (variantSize <= 0) return MusicSelectionState::Error;
            variantIndex = (variantIndex + variantSize - 1) % variantSize;
            break;
        }
        default:
            return MusicSelectionState::OutOfFunction;
    }
    return MusicSelectionState::Success;
}

MusicSelectionState MusicSelectionCursor::GetState() const
{
    return (manager->IsReloading()) ? MusicSelectionState::Reloading : state;
}

int32_t MusicSelectionCursor::GetCategorySize() const
{
    return manager->IsReloading() ? -1 : SU_TO_INT32(manager->categories.size());
}

int32_t MusicSelectionCursor::GetMusicSize(int32_t relativeIndex) const
{
    const auto category = GetCategoryAt(relativeIndex);
    return (category) ? SU_TO_INT32(category->Musics.size()) : -1;
}

int32_t MusicSelectionCursor::GetVariantSize(int32_t relativeIndex) const
{
    const auto music = GetMusicAt(relativeIndex);
    return (music) ? SU_TO_INT32(music->Scores.size()) : -1;
}

void MusicSelectionCursor::RegisterScriptInterface(asIScriptEngine *engine)
{
    engine->RegisterEnum(SU_IF_MSCSTATE);
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "OutOfFunction", int(MusicSelectionState::OutOfFunction));
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "Category", int(MusicSelectionState::Category));
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "Music", int(MusicSelectionState::Music));
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "Confirmed", int(MusicSelectionState::Confirmed));
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "Reloading", int(MusicSelectionState::Reloading));
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "Error", int(MusicSelectionState::Error));
    engine->RegisterEnumValue(SU_IF_MSCSTATE, "Success", int(MusicSelectionState::Success));

    engine->RegisterObjectType(SU_IF_MSCURSOR, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_MSCURSOR, asBEHAVE_ADDREF, "void f()", asMETHOD(MusicSelectionCursor, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_MSCURSOR, asBEHAVE_RELEASE, "void f()", asMETHOD(MusicSelectionCursor, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " ReloadMusic(bool = false)", asMETHOD(MusicSelectionCursor, ReloadMusic), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " ResetState()", asMETHOD(MusicSelectionCursor, ResetState), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetPrimaryString(int = 0)", asMETHOD(MusicSelectionCursor, GetPrimaryString), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetCategoryName(int = 0)", asMETHOD(MusicSelectionCursor, GetCategoryName), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetMusicName(int = 0)", asMETHOD(MusicSelectionCursor, GetMusicName), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetArtistName(int = 0)", asMETHOD(MusicSelectionCursor, GetArtistName), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetMusicJacketFileName(int = 0)", asMETHOD(MusicSelectionCursor, GetMusicJacketFileName), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetBackgroundFileName(int = 0)", asMETHOD(MusicSelectionCursor, GetBackgroundFileName), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetDifficulty(int = 0)", asMETHOD(MusicSelectionCursor, GetDifficulty), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetLevel(int = 0)", asMETHOD(MusicSelectionCursor, GetLevel), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "double GetBpm(int = 0)", asMETHOD(MusicSelectionCursor, GetBpm), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetExtraLevel(int = 0)", asMETHOD(MusicSelectionCursor, GetExtraLevel), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "string GetDesignerName(int = 0)", asMETHOD(MusicSelectionCursor, GetDesignerName), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Next()", asMETHOD(MusicSelectionCursor, Next), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Previous()", asMETHOD(MusicSelectionCursor, Previous), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " NextVariant()", asMETHOD(MusicSelectionCursor, NextVariant), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " PreviousVariant()", asMETHOD(MusicSelectionCursor, PreviousVariant), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Enter()", asMETHOD(MusicSelectionCursor, Enter), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " Exit()", asMETHOD(MusicSelectionCursor, Exit), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, SU_IF_MSCSTATE " GetState()", asMETHOD(MusicSelectionCursor, GetState), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetCategorySize()", asMETHOD(MusicSelectionCursor, GetCategorySize), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetMusicSize(int = 0)", asMETHOD(MusicSelectionCursor, GetMusicSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_MSCURSOR, "int GetVariantSize(int = 0)", asMETHOD(MusicSelectionCursor, GetVariantSize), asCALL_THISCALL);
}
