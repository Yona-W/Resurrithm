#pragma once

#define SU_NOTE_LONG_MASK  0b00000000001110000000
#define SU_NOTE_SHORT_MASK 0b00000000000001111110

enum class SusNoteType : uint16_t {
    Undefined = 0,

    // ショート
    Tap,            // Tap
    ExTap,          // ExTap
    Flick,          // Flick
    Air,            // Air
    HellTap,        // AIR: Hell Tap
    AwesomeExTap,   // STAR PLUS: やべーExTap (https://twitter.com/chunithm/status/967959264055648256)

    // ロング
    Hold = 7,       // Hold
    Slide,          // Slide
    AirAction,      // AirAction

    // 位置(ロング用)
    Start = 10,     // 開始
    Control,        // 変曲
    Step,           // 中継
    End,            // 終了

    // 方向(Air用、組み合わせ可)
    Up = 14,        // 上
    Down,           // 下
    Left,           // 左
    Right,          // 右

    // その他
    Injection = 18, // (ロング)コンボ挿入
    Invisible,      // 不可視
    MeasureLine,    // 小節線
    Grounded,       // Airの足が個別に描画される
    StartPosition,  // レーン奥での開始位置(オンゲキ用)
};

struct SusRelativeNoteTime {
    uint32_t Measure;
    uint32_t Tick;

    bool operator<(const SusRelativeNoteTime& b) const
    {
        return Measure < b.Measure || (Measure == b.Measure && Tick < b.Tick);
    }
    bool operator>(const SusRelativeNoteTime& b) const
    {
        return Measure > b.Measure || (Measure == b.Measure && Tick > b.Tick);
    }
    bool operator==(const SusRelativeNoteTime& b) const
    {
        return Measure == b.Measure && Tick == b.Tick;
    }
    bool operator!=(const SusRelativeNoteTime& b) const
    {
        return Measure != b.Measure || Tick != b.Tick;
    }
};


struct SusHispeedData {
    enum class Visibility {
        Keep = -1,
        Invisible,
        Visible,
    };
    const static double keepSpeed;

    SusHispeedData()
        : VisibilityState(Visibility::Visible)
        , Speed(1.0)
    {}

    SusHispeedData(
        const Visibility visibilityState,
        const double speed)
        : VisibilityState(visibilityState)
        , Speed(speed)
    {}

    Visibility VisibilityState;
    double Speed;

};

class SusHispeedTimeline final {
private:
    std::vector<std::pair<SusRelativeNoteTime, SusHispeedData>> keys;
    std::vector<std::tuple<double, double, SusHispeedData>> data;
    std::function<double(uint32_t, uint32_t)> relToAbs;

public:
    SusHispeedTimeline(std::function<double(uint32_t, uint32_t)> func);
    void AddKeysByString(const std::string &def, const std::function<std::shared_ptr<SusHispeedTimeline>(uint32_t)>& resolver);
    void AddKeyByData(uint32_t meas, uint32_t tick, double hs);
    void AddKeyByData(uint32_t meas, uint32_t tick, bool vis);
    void Finialize();
    std::tuple<bool, double> GetRawDrawStateAt(double time);
    double GetSpeedAt(double time);
};

class SusNoteExtraAttribute final {
public:
    uint32_t Priority = 0;
    int32_t RollHispeedNumber = -1;
    std::shared_ptr<SusHispeedTimeline> RollTimeline = nullptr;
    double HeightScale = 1.0;

    void Apply(const std::string &props);
};

enum class SusMetaDataFlags : uint16_t {
    DisableMetronome,
    EnableDrawPriority,
    EnableMovingLane,
};

struct SusMetaData {
    std::string UTitle = u8"";
    std::string USubTitle = u8"";
    std::string UArtist = u8"";
    std::string UJacketFileName = u8"";
    std::string UDesigner = u8"";
    std::string USongId = u8"";
    std::string UWaveFileName = u8"";
    std::string UBackgroundFileName = u8"";
    std::string UMovieFileName = u8"";
    double WaveOffset = 0;
    double MovieOffset = 0;
    double BaseBpm = 0;
    double ShowBpm = -1;
    double ScoreDuration = 0;
    int SegmentsPerSecond = 20;
    uint32_t Level = 0;
    uint32_t DifficultyType = 0;
    std::string UExtraDifficulty = "";
    std::bitset<8> ExtraFlags;

    void Reset()
    {
        USongId = "";
        UTitle = USubTitle = u8"";
        UArtist = UDesigner = u8"";
        UJacketFileName = UWaveFileName = UBackgroundFileName = "";
        WaveOffset = 0;
        BaseBpm = 0;
        Level = 0;
        ShowBpm = -1;
        ScoreDuration = 0;
        SegmentsPerSecond = 100;
        DifficultyType = 0;
        UExtraDifficulty = u8"";
        ExtraFlags.reset();
    }
};

struct SusRawNoteData {
    std::bitset<32> Type;
    std::shared_ptr<SusHispeedTimeline> Timeline;
    union {
        uint16_t DefinitionNumber = 0;
        struct {
            uint8_t StartLane;
            uint8_t Length;
        } NotePosition;
    };
    uint32_t Extra = 0;
    std::shared_ptr<SusNoteExtraAttribute> ExtraAttribute;

    bool operator==(const SusRawNoteData& b) const
    {
        if (Type != b.Type) return false;
        if (Timeline != b.Timeline) return false;
        if (DefinitionNumber != b.DefinitionNumber) return false;
        if (Extra != b.Extra) return false;
        if (ExtraAttribute != b.ExtraAttribute) return false;
        return true;
    }
    bool operator!=(const SusRawNoteData& b) const
    {
        return !(*this == b);
    }
};

struct SusDrawableNoteData {
    // SusRawNoteData からそのまま引き継ぐ奴ら
    std::bitset<32> Type;
    std::shared_ptr<SusHispeedTimeline> Timeline;
    std::shared_ptr<SusNoteExtraAttribute> ExtraAttribute;

    // それ以外
    std::bitset<8> OnTheFlyData;

    float StartLane = 0;  // ノーツ左端位置
    float Length = 0;     // ノーツ幅
    float CenterAtZero = 0; // ノーツ中心

    //実描画位置
    double ModifiedPosition = 0;
    double StartTimeEx = 0;
    double DurationEx = 0;
    //描画"始める"時刻
    double StartTime = 0;
    //描画が"続く"時刻
    double Duration = 0;
    //スライド・AA用制御データ
    std::vector<std::shared_ptr<SusDrawableNoteData>> ExtraData;

    std::tuple<bool, double> GetStateAt(double time);
};


using DrawableNotesList = std::vector<std::shared_ptr<SusDrawableNoteData>>;
using NoteCurvesList = std::unordered_map<std::shared_ptr<SusDrawableNoteData>, std::vector<std::tuple<double, double>>>;

// BMS派生フォーマットことSUS(SeaUrchinScore)の解析
class SusAnalyzer final {
private:
    static std::regex regexSusCommand;
    static std::regex regexSusData;

    const float defaultBeats = 4.0;
    const double defaultBpm = 120.0;
    const uint32_t defaultHispeedNumber = std::numeric_limits<uint32_t>::max();
    const uint32_t defaultExtraAttributeNumber = std::numeric_limits<uint32_t>::max();

    std::vector<std::function<void(std::string, std::string)>> errorCallbacks;
    const std::function<std::shared_ptr<SusHispeedTimeline>(uint32_t)> timelineResolver;

    uint32_t ticksPerBeat;          // 1拍あたりの分割数(分解能)
    uint32_t measureCountOffset;    // SUSデータから読み込んだ小節数に加算するオフセット
    uint32_t longNoteChannelOffset; // SUSデータから読み込んだロングノーツ識別番号に加算するオフセット
    double longInjectionPerBeat;    // 1拍あたりのロングノーツのカウント(コンボ)数

    std::vector<std::tuple<SusRelativeNoteTime, SusRawNoteData>> notes; // BPM指定、小節線、ノーツデータ全部入ってる

    std::unordered_map<uint32_t, double> bpmDefinitions;
    std::unordered_map<uint32_t, float> beatsDefinitions;
    std::unordered_map<uint32_t, std::shared_ptr<SusHispeedTimeline>> hispeedDefinitions;
    std::unordered_map<uint32_t, std::shared_ptr<SusNoteExtraAttribute>> extraAttributes;

    std::vector<std::tuple<SusRelativeNoteTime, SusRawNoteData>> bpmChanges; // notesからBPM指定だけコピーしてきて使う

    std::shared_ptr<SusHispeedTimeline> hispeedToApply, hispeedToMeasure;
    std::shared_ptr<SusNoteExtraAttribute> extraAttributeToApply;

    void ProcessCommand(const std::smatch &result, bool onlyMeta, uint32_t line);
    void ProcessRequest(const std::string &cmd, uint32_t line);
    void ProcessData(const std::smatch &result, uint32_t line);
    void MakeMessage(const std::string &message) const;
    void MakeMessage(uint32_t line, const std::string &message) const;
    void MakeMessage(uint32_t meas, uint32_t tick, uint32_t lane, const std::string &message) const;
    void CalculateCurves(const std::shared_ptr<SusDrawableNoteData>& note, NoteCurvesList &curveData) const;
    uint32_t GetMeasureCount(uint32_t relativeMeasureCount) const;
    uint32_t GetLongNoteChannel(uint32_t relativeLongNoteChannel) const;

public:
    SusMetaData SharedMetaData;
    std::vector<std::tuple<double, double>> SharedBpmChanges;

    explicit SusAnalyzer(uint32_t tpb);
    ~SusAnalyzer();

    void Reset();
    void SetMessageCallBack(const std::function<void(std::string, std::string)>& func);
    void LoadFromFile(const std::wstring &fileName, bool analyzeOnlyMetaData = false);
    void RenderScoreData(DrawableNotesList &data, NoteCurvesList &curveData);
    float GetBeatsAt(uint32_t measure) const;
    double GetBpmAt(uint32_t measure, uint32_t tick) const;
    double GetAbsoluteTime(uint32_t meas, uint32_t tick) const;
    std::tuple<uint32_t, uint32_t> GetRelativeTime(double time) const;
    uint32_t GetRelativeTicks(uint32_t measure, uint32_t tick) const;
    std::tuple<uint32_t, uint32_t> NormalizeRelativeTime(uint32_t meas, uint32_t tick) const;
};
