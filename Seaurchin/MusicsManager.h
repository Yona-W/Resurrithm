#pragma once

#define SU_IF_MSCURSOR "MusicCursor"
#define SU_IF_MSCSTATE "CursorState"

struct MusicScoreInfo final {
    uint16_t Difficulty = 0;
    uint16_t Level = 0;
    double BpmToShow = 0.0;
    std::string DifficultyName;
    std::string Designer;
    std::filesystem::path Path;
    std::filesystem::path WavePath;
    std::filesystem::path BackgroundPath;
};

struct MusicMetaInfo final {
    std::string SongId;
    std::string Name;
    std::string Artist;
    std::filesystem::path JacketPath;
    std::vector<std::shared_ptr<MusicScoreInfo>> Scores;
};


class CategoryInfo final {
private:

    std::string name;
    std::filesystem::path categoryPath;

public:
    std::vector<std::shared_ptr<MusicMetaInfo>> Musics;

    explicit CategoryInfo(const std::filesystem::path& cpath);
    ~CategoryInfo();

    std::string GetName() const { return name; }
    void Reload(bool recreateCache) const;
};

enum class MusicSelectionState {
    OutOfFunction = 0,
    Category,
    Music,
    Confirmed,

    Reloading,
    Error,
    Success,
};

class MusicSelectionCursor;
class ExecutionManager;
class SusAnalyzer;
class MusicsManager final {
    friend class MusicSelectionCursor;
private:
    ExecutionManager *manager;

    bool loading = false;
    std::mutex flagMutex;
    std::unique_ptr<SusAnalyzer> analyzer;
    std::vector<std::shared_ptr<CategoryInfo>> categories;
    void CreateMusicCache();

public:
    explicit MusicsManager(ExecutionManager *exm);
    ~MusicsManager();

    static void Initialize();
    void Reload(bool async);
    bool IsReloading();
    std::filesystem::path GetSelectedScorePath();

    MusicSelectionCursor *CreateMusicSelectionCursor();
    const std::vector<std::shared_ptr<CategoryInfo>> &GetCategories() const { return categories; }
};

class MusicSelectionCursor final {
    friend class MusicsManager;
private:
    int refcount = 0;

    MusicsManager *manager;
    int32_t categoryIndex;
    int32_t musicIndex;
    uint16_t variantIndex;
    MusicSelectionState state;

    std::shared_ptr<CategoryInfo> GetCategoryAt(int32_t relative) const;
    std::shared_ptr<MusicMetaInfo> GetMusicAt(int32_t relative) const;
    std::shared_ptr<MusicScoreInfo> GetScoreVariantAt(int32_t relative) const;

public:
    MusicSelectionCursor(MusicsManager *manager);
    void AddRef() { refcount++; }
    void Release() { if (--refcount == 0) delete this; }
    int GetRefCount() const { return refcount; }

    MusicSelectionState ReloadMusic(bool async);
    MusicSelectionState ResetState();

    std::string GetPrimaryString(int32_t relativeIndex) const;
    std::string GetCategoryName(int32_t relativeIndex) const;
    std::string GetMusicName(int32_t relativeIndex) const;
    std::string GetArtistName(int32_t relativeIndex) const;
    std::string GetMusicJacketFileName(int32_t relativeIndex) const;
    std::string GetBackgroundFileName(int32_t relativeIndex) const;
    int GetDifficulty(int32_t relativeIndex) const;
    int GetLevel(int32_t relativeIndex) const;
    double GetBpm(int32_t relativeIndex) const;
    std::string GetExtraLevel(int32_t relativeIndex) const;
    std::string GetDesignerName(int32_t relativeIndex) const;

    MusicSelectionState Enter();
    MusicSelectionState Exit();
    MusicSelectionState Next();
    MusicSelectionState Previous();
    MusicSelectionState NextVariant();
    MusicSelectionState PreviousVariant();
    MusicSelectionState GetState() const;

    int32_t GetCategorySize() const;
    int32_t GetMusicSize(int32_t relativeIndex) const;
    int32_t GetVariantSize(int32_t relativeIndex) const;

    static void RegisterScriptInterface(asIScriptEngine *engine);
};

struct MusicRawData final {
    std::string SongId;
    std::string Name;
    std::string Artist;
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> Scores;
};
