#include "SusAnalyzer.h"
#include <utility>
#include "Misc.h"

#include "Crc32.h"
#include <boost/algorithm/string.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <glm.hpp>

using namespace std;
using namespace crc32_constexpr;
namespace b = boost;
namespace ba = boost::algorithm;
namespace xp = boost::xpressive;

xp::sregex SusAnalyzer::regexSusCommand = "#" >> (xp::s1 = +xp::alnum) >> !(+xp::space >> (xp::s2 = +(~xp::_n)));
xp::sregex SusAnalyzer::regexSusData = "#" >> (xp::s1 = xp::repeat<3, 3>(xp::alnum)) >> (xp::s2 = xp::repeat<2, 3>(xp::alnum)) >> ":" >> *xp::space >> (xp::s3 = +(~xp::_n));

static xp::sregex allNumeric = xp::bos >> +(xp::digit) >> xp::eos;

auto toUpper = [](const char c) {
    return (c >= 'a' && c <= 'z') ? char(c - 0x20) : c;
};

static auto convertRawString = [](const string &input) -> string {
    // TIL: ASCII文字範囲ではUTF-8と本来のASCIIを間違うことはない
    if (ba::starts_with(input, "\"")) {
        ostringstream result;
        auto rest = input;
        trim_if(rest, ba::is_any_of("\""));
        auto it = rest.begin();
        while (it != rest.end()) {
            if (*it != '\\') {
                result << *it;
                ++it;
                continue;
            }
            ++it;
            if (it == rest.end()) return "";
            switch (*it) {
                case '"':
                    result << "\"";
                    break;
                case 't':
                    result << "\t";
                    break;
                case 'n':
                    result << "\n";
                    break;
                case 'u': {
                    /*
                    //utf-8 4byte食う
                    char cp[5] = { 0 };
                    for (auto i = 0; i < 4; i++) {
                        cp[i] = *(++it);
                    }
                    */
                    // wchar_t r = stoi(cp, 0, 16);
                    //でも突っ込むのめんどくさいので🙅で代用します
                    result << "u";
                    break;
                }
                default:
                    break;
            }
            ++it;
        }
        return result.str();
    }
    return input;
};

SusAnalyzer::SusAnalyzer(const uint32_t tpb)
    : timelineResolver([=](const uint32_t number) { return hispeedDefinitions[number]; })
    , ticksPerBeat(tpb)
    , measureCountOffset(0)
    , longNoteChannelOffset(0)
    , longInjectionPerBeat(2)
{}

SusAnalyzer::~SusAnalyzer()
{
    Reset();
}

void SusAnalyzer::Reset()
{
    errorCallbacks.clear();
    errorCallbacks.emplace_back([](auto type, auto message) {
        auto log = spdlog::get("main");
        log->error(message);
    });

    ticksPerBeat = 192;
    longInjectionPerBeat = 2;
    measureCountOffset = 0;
    longNoteChannelOffset = 0;

    notes.clear();

    bpmDefinitions.clear();
    beatsDefinitions.clear();
    hispeedDefinitions.clear();
    extraAttributes.clear();

    bpmChanges.clear();

    SharedMetaData.Reset();
    SharedBpmChanges.clear();

    bpmDefinitions[1] = 120.0;
    beatsDefinitions[0] = 4.0;

    const auto defhs = make_shared<SusHispeedTimeline>([&](const uint32_t m, const uint32_t t) { return GetAbsoluteTime(m, t); });
    defhs->AddKeysByString("0'0:1.0:v", timelineResolver);
    hispeedDefinitions[defaultHispeedNumber] = defhs;
    hispeedToApply = defhs;
    hispeedToMeasure = defhs;

    const auto defea = make_shared<SusNoteExtraAttribute>();
    defea->Priority = 0;
    defea->HeightScale = 1;
    extraAttributes[defaultExtraAttributeNumber] = defea;
    extraAttributeToApply = defea;
}

void SusAnalyzer::SetMessageCallBack(const function<void(string, string)>& func)
{
    errorCallbacks.push_back(func);
}

//一応UTF-8として処理することにしますがどうせ変わらないだろうなぁ
//あと列挙済みファイルを流し込む前提でエラーチェックしない
void SusAnalyzer::LoadFromFile(const string &fileName, const bool analyzeOnlyMetaData)
{
    auto log = spdlog::get("main");
    ifstream file;
    string rawline;
    xp::smatch match;
    uint32_t line = 0;

    Reset();
    if (!analyzeOnlyMetaData) log->info(u8"Parsing {0}", fileName);

    file.open(fileName, ios::in);

    char bom[3] = { 0 };
    file.read(bom, 3);
    if (file.gcount() != 3 || bom[0] != char(0xEF) || bom[1] != char(0xBB) || bom[2] != char(0xBF)) file.seekg(0);

    while (getline(file, rawline)) {
        ++line;
        if (rawline.empty() || rawline[0] != '#') continue;

        if (xp::regex_match(rawline, match, regexSusCommand)) {
            ProcessCommand(match, analyzeOnlyMetaData, line);
        } else if (xp::regex_match(rawline, match, regexSusData)) {
            if (!analyzeOnlyMetaData || boost::starts_with(rawline, "#BPM")) ProcessData(match, line);
        } else {
            MakeMessage(line, "SUS line valid but could not be parsed");
        }
    }
    file.close();

    if (!analyzeOnlyMetaData) log->info(u8"End");
    if (!analyzeOnlyMetaData) {
        // いい感じにソート
        stable_sort(notes.begin(), notes.end(), [](tuple<SusRelativeNoteTime, SusRawNoteData> a, tuple<SusRelativeNoteTime, SusRawNoteData> b) {
            const auto &at = get<0>(a);
            const auto &bt = get<0>(b);

            return at < bt || (at == bt && get<1>(a).Type.to_ulong() > get<1>(b).Type.to_ulong());
        });

        // 小節線ノーツ
        // この時点でケツは最終ノーツのはず
        const auto lastMeasure = get<0>(notes[notes.size() - 1]).Measure + 2;
        for (auto i = 0u; i <= lastMeasure; i++) {
            SusRawNoteData ml;
            SusRelativeNoteTime t = { i, 0 };
            ml.Type.set(size_t(SusNoteType::MeasureLine));
            ml.Timeline = hispeedToMeasure;
            ml.ExtraAttribute = extraAttributes[defaultExtraAttributeNumber];
            notes.emplace_back(t, ml);
        }

        copy_if(notes.begin(), notes.end(), back_inserter(bpmChanges), [](tuple<SusRelativeNoteTime, SusRawNoteData> n) {
            return get<1>(n).Type.test(size_t(SusNoteType::Undefined));
        });
        if (bpmChanges.empty()) {
            SusRawNoteData noteData;
            SusRelativeNoteTime time = { 0, 0 };
            noteData.Type.set(size_t(SusNoteType::Undefined));
            noteData.DefinitionNumber = 1; // リセット時にbpmDefinitions[1]が設定されていることより、1は必ず有効であると仮定している。(0 basedじゃないのはなぜ?)
            bpmChanges.emplace_back(time, noteData);
        }

        for (auto &hs : hispeedDefinitions) hs.second->Finialize();
        if (SharedMetaData.BaseBpm == 0) SharedMetaData.BaseBpm = GetBpmAt(0, 0);
        for (const auto& bpm : bpmChanges) {
            SusRelativeNoteTime t = {};
            SusRawNoteData d = {};
            tie(t, d) = bpm;
            SharedBpmChanges.emplace_back(GetAbsoluteTime(t.Measure, t.Tick), bpmDefinitions[d.DefinitionNumber]);
        }
    }
}

void SusAnalyzer::ProcessCommand(const xp::smatch &result, const bool onlyMeta, const uint32_t line)
{
    auto name = result[1].str();
    transform(name.cbegin(), name.cend(), name.begin(), toUpper);
    switch (Crc32Rec(0xffffffff, name.c_str())) {
        case "TITLE"_crc32:
            SharedMetaData.UTitle = convertRawString(result[2]);
            break;
        case "SUBTITLE"_crc32:
            SharedMetaData.USubTitle = convertRawString(result[2]);
            break;
        case "ARTIST"_crc32:
            SharedMetaData.UArtist = convertRawString(result[2]);
            break;
        case "GENRE"_crc32:
            // SharedMetaData.UGenre = ConvertRawString(result[2]);
            break;
        case "DESIGNER"_crc32:
        case "SUBARTIST"_crc32:  // BMS互換
            SharedMetaData.UDesigner = convertRawString(result[2]);
            break;
        case "PLAYLEVEL"_crc32: {
            if (SharedMetaData.DifficultyType == 4) {
                MakeMessage(line, "Level field unused for WORLD'S END");
                break;
            }

            string lstr = result[2];
            const auto pluspos = lstr.find('+');
            if (pluspos != string::npos) {
                SharedMetaData.UExtraDifficulty = "+";
                SharedMetaData.Level = ConvertInteger(lstr.substr(0, pluspos));
            } else {
                SharedMetaData.UExtraDifficulty = "";
                SharedMetaData.Level = ConvertInteger(lstr);
            }
            break;
        }
        case "DIFFICULTY"_crc32: {
            if (xp::regex_match(result[2], allNumeric)) {
                //通常記法
                const auto difficultyType = ConvertInteger(result[2]);
                if (difficultyType < 0 || 3 < difficultyType) {
                    MakeMessage(line, "Difficulty is not in valid range");
                    break;
                }

                // 複数回PLAYLEVEL、DIFFICULTY指定しないでくれって話ではあるけど
                // WE指定した後に再度通常の難易度指定したならLevel、UExtraDifficultyはパラメータとして互換性ないので初期化
                // 通常の難易度指定した後に再度通常の難易度指定したならパラメータを引き継ぐ
                if (SharedMetaData.DifficultyType == 4) {
                    SharedMetaData.Level = 0;
                    SharedMetaData.UExtraDifficulty = "";
                }
                SharedMetaData.DifficultyType = difficultyType;
            } else {
                //WE記法
                auto dd = convertRawString(result[2]);
                vector<string> params;
                ba::split(params, dd, ba::is_any_of(":"));
                if (params.size() < 2) {
                    MakeMessage(line, "Difficulty format incorrect");
                    return;
                }
                SharedMetaData.DifficultyType = 4;
                SharedMetaData.Level = ConvertInteger(params[0]); // 星の数として扱う
                SharedMetaData.UExtraDifficulty = params[1]; // 難易度文字として扱う
            }
            break;
        }
        case "SONGID"_crc32:
            SharedMetaData.USongId = convertRawString(result[2]);
            break;
        case "WAVE"_crc32:
            SharedMetaData.UWaveFileName = convertRawString(result[2]);
            break;
        case "WAVEOFFSET"_crc32:
            SharedMetaData.WaveOffset = ConvertFloat(result[2]);
            break;
        case "MOVIE"_crc32:
            SharedMetaData.UMovieFileName = convertRawString(result[2]);
            break;
        case "MOVIEOFFSET"_crc32:
            SharedMetaData.MovieOffset = ConvertFloat(result[2]);
            break;
        case "JACKET"_crc32:
            SharedMetaData.UJacketFileName = convertRawString(result[2]);
            break;
        case "BACKGROUND"_crc32:
            SharedMetaData.UBackgroundFileName = convertRawString(result[2]);
            break;
        case "REQUEST"_crc32:
            ProcessRequest(convertRawString(result[2]), line);
            break;
        case "BASEBPM"_crc32:
            SharedMetaData.BaseBpm = ConvertFloat(result[2]);
            break;

            //此処から先はデータ内で使う用
        case "HISPEED"_crc32: {
            if (onlyMeta) break;
            const auto hsn = ConvertHexatridecimal(result[2]);
            if (hispeedDefinitions.find(hsn) == hispeedDefinitions.end()) {
                MakeMessage(line, "Timeline does not exist");
                break;
            }
            hispeedToApply = hispeedDefinitions[hsn];
            break;
        }
        case "NOSPEED"_crc32:
            if (onlyMeta) break;

            hispeedToApply = hispeedDefinitions[defaultHispeedNumber];
            break;

        case "ATTRIBUTE"_crc32: {
            if (onlyMeta) break;

            const auto ean = ConvertHexatridecimal(result[2]);
            if (extraAttributes.find(ean) == extraAttributes.end()) {
                MakeMessage(line, "Attribute does not exist");
                break;
            }
            extraAttributeToApply = extraAttributes[ean];
            break;
        }
        case "NOATTRIBUTE"_crc32:
            if (onlyMeta) break;

            extraAttributeToApply = extraAttributes[defaultExtraAttributeNumber];
            break;

        case "MEASUREHS"_crc32: {
            if (onlyMeta) break;

            const auto hsn = ConvertHexatridecimal(result[2]);
            if (hispeedDefinitions.find(hsn) == hispeedDefinitions.end()) {
                MakeMessage(line, "Timeline does not exist");
                break;
            }
            hispeedToMeasure = hispeedDefinitions[hsn];
            break;
        }

        case "MEASUREBS"_crc32: {
            if (onlyMeta) break;

            const auto bsc = ConvertInteger(result[2]);
            if (bsc < 0) {
                MakeMessage(line, "Invalid measure offset");
                break;
            }
            measureCountOffset = bsc;
            break;
        }

        case "CHANNELBS"_crc32: {
            if (onlyMeta) break;

            const auto bsc = ConvertInteger(result[2]);
            if (bsc < 0) {
                MakeMessage(line, "Invalid channel offset");
                break;
            }
            longNoteChannelOffset = bsc;
            break;
        }

        default:
            MakeMessage(line, "Invalid SUS command");
            break;
    }

}

void SusAnalyzer::ProcessRequest(const string &cmd, const uint32_t line)
{
    auto str = cmd;
    b::trim_if(str, ba::is_any_of(" "));
    vector<string> params;
    ba::split(params, str, ba::is_any_of(" "), b::token_compress_on);

    if (params.empty()) return;
    switch (Crc32Rec(0xffffffff, params[0].c_str())) {
        case "metronome"_crc32:
            SharedMetaData.ExtraFlags[size_t(SusMetaDataFlags::DisableMetronome)] = !ConvertBoolean(params[1]);
            break;
        case "ticks_per_beat"_crc32:
            ticksPerBeat = ConvertInteger(params[1]);
            break;
        case "enable_priority"_crc32:
            MakeMessage(line, "Setting priority");
            SharedMetaData.ExtraFlags[size_t(SusMetaDataFlags::EnableDrawPriority)] = ConvertBoolean(params[1]);
            break;
        case "enable_moving_lane"_crc32:
            MakeMessage(line, "Enabling lane movement");
            SharedMetaData.ExtraFlags[size_t(SusMetaDataFlags::EnableMovingLane)] = ConvertBoolean(params[1]);
            break;
        case "segments_per_second"_crc32:
            SharedMetaData.SegmentsPerSecond = ConvertInteger(params[1]);
            break;
        default:
            break;
    }
}

void SusAnalyzer::ProcessData(const xp::smatch &result, const uint32_t line)
{
    auto meas = result[1].str();
    auto lane = result[2].str();
    auto pattern = result[3].str();
    ba::erase_all(pattern, " ");

    /*
     判定順について
     0. #...** (BPMなど)
     1. #---0* (特殊データ、定義分割不可)
     2. #---1* (Short)
     3. #---5* (Air)
     4. #---[234]*. (Long)
    */

    const auto noteCount = pattern.length() / 2;
    const auto step = uint32_t(ticksPerBeat * GetBeatsAt(GetMeasureCount(ConvertInteger(meas)))) / (!noteCount ? 1 : noteCount);

    if (!regex_match(meas, allNumeric)) {
        // コマンドデータ
        transform(meas.cbegin(), meas.cend(), meas.begin(), toUpper);
        if (meas == "BPM") {
            const auto number = ConvertHexatridecimal(lane);
            const auto value = ConvertFloat(pattern);
            bpmDefinitions[number] = value;
            if (SharedMetaData.ShowBpm < 0) SharedMetaData.ShowBpm = value;
        } else if (meas == "TIL") {
            const auto number = ConvertHexatridecimal(lane);
            auto it = hispeedDefinitions.find(number);
            if (it == hispeedDefinitions.end()) {
                auto hs = make_shared<SusHispeedTimeline>([&](const uint32_t m, const uint32_t t) { return GetAbsoluteTime(m, t); });
                hs->AddKeysByString(convertRawString(pattern), timelineResolver);
                hispeedDefinitions[number] = hs;
            } else {
                it->second->AddKeysByString(convertRawString(pattern), timelineResolver);
            }
        } else if (meas == "ATR") {
            const auto number = ConvertHexatridecimal(lane);
            auto it = extraAttributes.find(number);
            if (it == extraAttributes.end()) {
                auto ea = make_shared<SusNoteExtraAttribute>();
                ea->Apply(convertRawString(pattern));
                extraAttributes[number] = ea;
            } else {
                it->second->Apply(convertRawString(pattern));
            }
        } else {
            MakeMessage(line, "Invalid data command");
        }
    } else if (lane[0] == '0') {
        switch (lane[1]) {
            case '2':
                // 小節長
                beatsDefinitions[GetMeasureCount(ConvertInteger(meas))] = ConvertFloat(pattern);
                break;
            case '8': {
                // BPM
                for (auto i = 0u; i < noteCount; i++) {
                    const auto note = pattern.substr(i * 2, 2);

                    SusRawNoteData noteData;
                    noteData.Type.set(size_t(SusNoteType::Undefined));
                    noteData.DefinitionNumber = ConvertHexatridecimal(note);
                    if (!noteData.DefinitionNumber) continue;

                    const SusRelativeNoteTime time = { GetMeasureCount(ConvertInteger(meas)), step * i };
                    notes.emplace_back(time, noteData);
                }
                break;
            }
            default:
                MakeMessage(line, "Invalid data command");
                break;
        }
    } else if (lane[0] == '1') {
        // ショートノーツ
        for (auto i = 0u; i < noteCount; i++) {
            const auto note = pattern.substr(i * 2, 2);

            SusRawNoteData noteData;
            noteData.NotePosition.StartLane = ConvertHexatridecimal(lane.substr(1, 1));
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Timeline = hispeedToApply;
            noteData.ExtraAttribute = extraAttributeToApply;

            switch (note[0]) {
                case '1':
                    noteData.Type.set(size_t(SusNoteType::Tap));
                    break;
                case '2':
                    noteData.Type.set(size_t(SusNoteType::ExTap));
                    break;
                case '3':
                    noteData.Type.set(size_t(SusNoteType::Flick));
                    break;
                case '4':
                    noteData.Type.set(size_t(SusNoteType::HellTap));
                    break;
                case '5':
                    noteData.Type.set(size_t(SusNoteType::AwesomeExTap));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    break;
                case '6':
                    noteData.Type.set(size_t(SusNoteType::AwesomeExTap));
                    noteData.Type.set(size_t(SusNoteType::Down));
                    break;
                default:
                    if (note[1] == '0') continue;
                    MakeMessage(line, "Invalid tap lane specification");
                    continue;
            }

            const SusRelativeNoteTime time = { GetMeasureCount(ConvertInteger(meas)), step * i };
            notes.emplace_back(time, noteData);
        }
    } else if (lane[0] == '5') {
        // Airノーツ
        for (auto i = 0u; i < noteCount; i++) {
            const auto note = pattern.substr(i * 2, 2);

            SusRawNoteData noteData;
            noteData.NotePosition.StartLane = ConvertHexatridecimal(lane.substr(1, 1));
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Timeline = hispeedToApply;
            noteData.ExtraAttribute = extraAttributeToApply;

            switch (note[0]) {
                case '1':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    break;
                case '2':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Down));
                    break;
                case '3':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    noteData.Type.set(size_t(SusNoteType::Left));
                    break;
                case '4':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    noteData.Type.set(size_t(SusNoteType::Right));
                    break;
                case '5':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Down));
                    noteData.Type.set(size_t(SusNoteType::Right));
                    break;
                case '6':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Down));
                    noteData.Type.set(size_t(SusNoteType::Left));
                    break;
                case '7':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    noteData.Type.set(size_t(SusNoteType::Grounded));
                    break;
                case '8':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    noteData.Type.set(size_t(SusNoteType::Left));
                    noteData.Type.set(size_t(SusNoteType::Grounded));
                    break;
                case '9':
                    noteData.Type.set(size_t(SusNoteType::Air));
                    noteData.Type.set(size_t(SusNoteType::Up));
                    noteData.Type.set(size_t(SusNoteType::Right));
                    noteData.Type.set(size_t(SusNoteType::Grounded));
                    break;
                default:
                    if (note[1] == '0') continue;
                    MakeMessage(line, "Invalid air lane specification");
                    continue;
            }

            const SusRelativeNoteTime time = { GetMeasureCount(ConvertInteger(meas)), step * i };
            notes.emplace_back(time, noteData);
        }
    } else if (lane.length() == 3 && lane[0] >= '2' && lane[0] <= '4') {
        // ロングタイプ
        for (auto i = 0u; i < noteCount; i++) {
            const auto note = pattern.substr(i * 2, 2);

            SusRawNoteData noteData;
            noteData.NotePosition.StartLane = ConvertHexatridecimal(lane.substr(1, 1));
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Extra = GetLongNoteChannel(ConvertHexatridecimal(lane.substr(2, 1)));
            noteData.Timeline = hispeedToApply;
            noteData.ExtraAttribute = extraAttributeToApply;

            switch (lane[0]) {
                case '2':
                    noteData.Type.set(size_t(SusNoteType::Hold));
                    break;
                case '3':
                    noteData.Type.set(size_t(SusNoteType::Slide));
                    break;
                case '4':
                    noteData.Type.set(size_t(SusNoteType::AirAction));
                    break;
                default:
                    MakeMessage(line, "Invalid hold lane specification");
                    continue;
            }
            switch (note[0]) {
                case '1':
                    noteData.Type.set(size_t(SusNoteType::Start));
                    break;
                case '2':
                    noteData.Type.set(size_t(SusNoteType::End));
                    break;
                case '3':
                    noteData.Type.set(size_t(SusNoteType::Step));
                    break;
                case '4':
                    noteData.Type.set(size_t(SusNoteType::Control));
                    break;
                case '5':
                    noteData.Type.set(size_t(SusNoteType::Invisible));
                    break;
                default:
                    if (note[1] == '0') continue;
                    MakeMessage(line, "Invalid note type");
                    continue;
            }

            const SusRelativeNoteTime time = { GetMeasureCount(ConvertInteger(meas)), step * i };
            notes.emplace_back(time, noteData);
        }
    } else if (lane.length() == 3 && lane[0] == '9') {
        // z = 0のレーン指定(Tap) #mmm800~#mmm80f~#mmm8ff
        const auto endlane = lane.substr(1, 1);
        const auto startlane = lane.substr(2, 1);

        for (auto i = 0u; i < noteCount; i++) {
            const auto note = pattern.substr(i * 2, 2);
            if (note[1] == '0') continue;
            if (note[0] != '1') {
                MakeMessage(line, "Invalid start lane");
                continue;
            }

            SusRawNoteData noteData;
            noteData.NotePosition.StartLane = ConvertHexatridecimal(endlane);
            noteData.Extra = ConvertHexatridecimal(startlane);
            noteData.NotePosition.Length = ConvertHexatridecimal(note.substr(1, 1));
            noteData.Type.set(size_t(SusNoteType::StartPosition));
            // noteData.Timeline = hispeedToApply;
            // noteData.ExtraAttribute = extraAttributeToApply;

            const SusRelativeNoteTime time = { GetMeasureCount(ConvertInteger(meas)), step * i };
            notes.emplace_back(time, noteData);
        }
    } else {
        // 不正
        MakeMessage(line, "Invalid data definition");
    }
}

void SusAnalyzer::MakeMessage(const string &message) const
{
    for (const auto &cb : errorCallbacks) cb("Error", message);
}

void SusAnalyzer::MakeMessage(const uint32_t line, const string &message) const
{
    ostringstream ss;
    ss << "Line " << line << ": " << message;
    MakeMessage(ss.str());
}

void SusAnalyzer::MakeMessage(const uint32_t meas, const uint32_t tick, const uint32_t lane, const std::string &message) const
{
    ostringstream ss;
    ss << meas << "'" << tick << "@" << lane << ": " << message;
    MakeMessage(ss.str());
}

float SusAnalyzer::GetBeatsAt(const uint32_t measure) const
{
    auto result = defaultBeats;
    auto last = 0u;
    for (auto &t : beatsDefinitions) {
        if (t.first >= last && t.first <= measure) {
            result = t.second;
            last = t.first;
        }
    }
    return result;
}

double SusAnalyzer::GetBpmAt(const uint32_t measure, const uint32_t tick) const
{
    uint32_t nm, nt;
    tie(nm, nt) = NormalizeRelativeTime(measure, tick);
    auto result = defaultBpm;
    auto lastMeasure = 0u;
    auto lastTick = 0u;
    for (const auto &t : bpmChanges) {
        const auto bcTime = get<0>(t);
        if (bcTime.Measure > nm || bcTime.Measure < lastMeasure) continue;
        if (bcTime.Measure == lastMeasure && lastTick > bcTime.Tick) continue;

        const auto &bpmDef = bpmDefinitions.find(get<1>(t).DefinitionNumber);
        if (bpmDef != bpmDefinitions.end()) {
            result = bpmDef->second;
        }
        lastMeasure = bcTime.Measure;
        lastTick = bcTime.Tick;
    }
    return result;
}

tuple<uint32_t, uint32_t> SusAnalyzer::NormalizeRelativeTime(const uint32_t meas, const uint32_t tick) const
{
    auto nm = meas;
    auto nt = tick;
    while (nt >= GetBeatsAt(nm) * ticksPerBeat) nt -= SU_TO_UINT32(GetBeatsAt(nm++) * ticksPerBeat);
    return make_tuple(nm, nt);
}


double SusAnalyzer::GetAbsoluteTime(const uint32_t meas, const uint32_t tick) const
{
    auto time = 0.0;
    auto lastBpm = defaultBpm;
    //超過したtick指定にも対応したほうが使いやすいよね
    uint32_t nm, nt;
    tie(nm, nt) = NormalizeRelativeTime(meas, tick);

    for (auto i = 0u; i < nm + 1; i++) {
        const auto beats = GetBeatsAt(i);
        auto lastChangeTick = 0u;
        for (auto& bc : bpmChanges) {
            if (get<0>(bc).Measure != i) continue;
            const auto timing = get<0>(bc);
            if (i == nm && timing.Tick >= nt) break;
            const auto dur = (60.0 / lastBpm) * (double(timing.Tick - lastChangeTick) / ticksPerBeat);
            time += dur;
            lastChangeTick = timing.Tick;
            const auto bpmDefinition = bpmDefinitions.find(get<1>(bc).DefinitionNumber);
            if (bpmDefinition != bpmDefinitions.end()) {
                lastBpm = bpmDefinition->second;
            }
        }
        if (i == nm) {
            time += (60.0 / lastBpm) * (double(nt - lastChangeTick) / ticksPerBeat);
        } else {
            time += (60.0 / lastBpm) * (double(ticksPerBeat * beats - lastChangeTick) / ticksPerBeat);
        }
    }

    return time;
}

tuple<uint32_t, uint32_t> SusAnalyzer::GetRelativeTime(const double time) const
{
    auto restTime = time;
    uint32_t meas = 0;
    auto secPerBeat = (60.0 / 120.0);

    while (true) {
        const auto beats = GetBeatsAt(meas);
        auto lastChangeTick = 0u;

        for (auto& bc : bpmChanges) {
            const auto timing = get<0>(bc);
            if (timing.Measure != meas) continue;
            const auto dur = secPerBeat * (double(timing.Tick - lastChangeTick) / ticksPerBeat);
            if (dur >= restTime) return make_tuple(meas, lastChangeTick + static_cast<uint32_t>(restTime / secPerBeat * ticksPerBeat));
            restTime -= dur;
            lastChangeTick = timing.Tick;
            const auto bpmDef = bpmDefinitions.find(get<1>(bc).DefinitionNumber);

            if (bpmDef != bpmDefinitions.end()) {
                secPerBeat = 60.0 / bpmDef->second;
            }
        }
        const double restTicks = ticksPerBeat * beats - lastChangeTick;
        const auto restDuration = restTicks / ticksPerBeat * secPerBeat;
        if (restDuration >= restTime) return make_tuple(meas, lastChangeTick + static_cast<uint32_t>(restTime / secPerBeat * ticksPerBeat));
        restTime -= restDuration;
        meas++;
    }
}

uint32_t SusAnalyzer::GetRelativeTicks(const uint32_t measure, const uint32_t tick) const
{
    float result = 0;
    for (auto i = 0u; i < measure; i++) result += GetBeatsAt(i) * ticksPerBeat;
    return SU_TO_UINT32(result) + tick;
}

void SusAnalyzer::RenderScoreData(DrawableNotesList &data, NoteCurvesList &curveData)
{
    // 不正チェックリスト
    // ショート: はみ出しは全部アウト
    // ホールド: ケツ無しアウト(ケツ連は無視)、Step/Control問答無用アウト、ケツ違いアウト
    // スライド、AA: ケツ無しアウト(ケツ連は無視)
    data.clear();
    for (const auto& note : notes) {
        const auto time = get<0>(note);
        const auto &info = get<1>(note);
        if (info.Type[size_t(SusNoteType::Step)]) continue;
        if (info.Type[size_t(SusNoteType::Control)]) continue;
        if (info.Type[size_t(SusNoteType::Invisible)]) continue;
        if (info.Type[size_t(SusNoteType::End)]) continue;
        if (info.Type[size_t(SusNoteType::Undefined)]) continue;

        if (info.NotePosition.StartLane + info.NotePosition.Length > 16) {
            MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, u8"Tap note endpoint exceeds playfield width");
            continue;
        }

        auto noteData = make_shared<SusDrawableNoteData>();
        noteData->Type = info.Type;
        noteData->Timeline = info.Timeline;
        noteData->ExtraAttribute = info.ExtraAttribute;
        noteData->StartTime = GetAbsoluteTime(time.Measure, time.Tick);
        noteData->Duration = 0;
        noteData->StartTimeEx = get<1>(noteData->Timeline->GetRawDrawStateAt(noteData->StartTime));
        noteData->StartLane = info.NotePosition.StartLane;
        noteData->Length = info.NotePosition.Length;
        noteData->CenterAtZero = noteData->StartLane + noteData->Length / 2.0f;

        const auto bits = info.Type.to_ulong();
        if (bits & SU_NOTE_LONG_MASK) {
            auto genCurve = true;
            SusNoteType ltype;
            switch ((bits >> 7) & 7) {
                case 1:
                    ltype = SusNoteType::Hold;
                    genCurve = false;
                    break;
                case 2:
                    ltype = SusNoteType::Slide;
                    break;
                case 4:
                    ltype = SusNoteType::AirAction;
                    break;
                default:
                    MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, "Internal representation error");
                    continue;
            }

            auto completed = false;
            auto lastStep = note;
            for (const auto &it : notes) {
                const auto curPos = get<0>(it);
                const auto &curNo = get<1>(it);

                if (curNo.Type.test(size_t(SusNoteType::Start))) continue;
                if (!curNo.Type.test(size_t(ltype)) || curNo.Extra != info.Extra) continue;
                if (curPos < time) continue;

                if (curNo.NotePosition.StartLane + curNo.NotePosition.Length > 16) {
                    MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, "Note endpoint exceeds playfield width");
                    continue;
                }

                switch (ltype) {
                    case SusNoteType::Hold: {
                        if (curNo.Type.test(size_t(SusNoteType::Control)) || curNo.Type.test(size_t(SusNoteType::Invisible))) {
                            MakeMessage(curPos.Measure, curPos.Tick, curNo.NotePosition.StartLane, "A note cannot be Control and Invisible at the same time");
                            continue;
                        }
                        if (curNo.DefinitionNumber != info.DefinitionNumber) continue;
                    }
                                            /* ホールドだけ追加チェックしてフォールスルー */
                    case SusNoteType::Slide:
                    case SusNoteType::AirAction: {
                        auto nextNote = make_shared<SusDrawableNoteData>();
                        nextNote->Type = curNo.Type;
                        nextNote->Timeline = curNo.Timeline;
                        nextNote->ExtraAttribute = curNo.ExtraAttribute;
                        nextNote->StartTime = GetAbsoluteTime(curPos.Measure, curPos.Tick);
                        nextNote->StartTimeEx = get<1>(nextNote->Timeline->GetRawDrawStateAt(nextNote->StartTime));
                        nextNote->StartLane = curNo.NotePosition.StartLane;
                        nextNote->Length = curNo.NotePosition.Length;
                        // 暫定
                        nextNote->CenterAtZero = nextNote->StartLane + nextNote->Length / 2.0f;

                        if (curNo.Type.test(size_t(SusNoteType::Step)) || curNo.Type.test(size_t(SusNoteType::End))) {
                            const auto lsrt = get<0>(lastStep);
                            const auto injc = double(GetRelativeTicks(curPos.Measure, curPos.Tick) - GetRelativeTicks(lsrt.Measure, lsrt.Tick)) / ticksPerBeat * longInjectionPerBeat;
                            for (auto i = 1; i < injc; i++) {
                                const auto insertAt = SU_TO_INT32(lsrt.Tick + (ticksPerBeat / longInjectionPerBeat * i));
                                auto injection = make_shared<SusDrawableNoteData>();
                                injection->Type.set(size_t(SusNoteType::Injection));
                                injection->StartTime = GetAbsoluteTime(lsrt.Measure, insertAt);
                                noteData->ExtraData.push_back(injection);
                            }
                        }

                        if (nextNote->Type.test(size_t(SusNoteType::Step))) lastStep = it;

                        if (curNo.Type.test(size_t(SusNoteType::End))) {
                            noteData->Duration = nextNote->StartTime - noteData->StartTime;
                            completed = true;
                        }
                        noteData->ExtraData.push_back(nextNote);
                        break;
                    }
                    default:
                        // 特に何もしない
                        break;
                }
                if (completed) break;
            }
            if (!completed) {
                MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, "Hold note does not have an endpoint");
                continue;
            }

            SharedMetaData.ScoreDuration = max(SharedMetaData.ScoreDuration, noteData->StartTime + noteData->Duration);
            if (genCurve) {
                CalculateCurves(noteData, curveData);
                // Injection の幅を決定する
                for (const auto &extra : noteData->ExtraData) {
                    if (!extra->Type[size_t(SusNoteType::Injection)]) continue;

                    auto last = noteData;
                    for (auto &slideElement : noteData->ExtraData) {
                        if (slideElement->Type.test(size_t(SusNoteType::Control))) continue;
                        if (slideElement->Type.test(size_t(SusNoteType::Injection))) continue;
                        if (extra->StartTime >= slideElement->StartTime) {
                            last = slideElement;
                            continue;
                        }
                        const auto width = glm::mix(
                            last->Length, slideElement->Length,
                            (extra->StartTime - last->StartTime) / (slideElement->StartTime - last->StartTime)
                        );
                        auto &segmentPositions = curveData[slideElement];

                        auto lastSegmentPosition = segmentPositions[0];
                        auto lastTimeInBlock = get<0>(lastSegmentPosition) / (slideElement->StartTime - last->StartTime);
                        for (auto &segmentPosition : segmentPositions) {
                            if (lastSegmentPosition == segmentPosition) continue;
                            const auto currentTimeInBlock = get<0>(segmentPosition) / (slideElement->StartTime - last->StartTime);
                            const auto cst = glm::mix(last->StartTime, slideElement->StartTime, currentTimeInBlock);
                            if (extra->StartTime >= cst) {
                                lastSegmentPosition = segmentPosition;
                                lastTimeInBlock = currentTimeInBlock;
                                continue;
                            }
                            const auto lst = glm::mix(last->StartTime, slideElement->StartTime, lastTimeInBlock);
                            const auto t = (extra->StartTime - lst) / (cst - lst);
                            const auto x = glm::mix(get<1>(lastSegmentPosition), get<1>(segmentPosition), t);
                            const auto center = x * 16.0;
                            extra->StartLane = SU_TO_FLOAT(center - width / 2.0);
                            extra->Length = width;
                            break;
                        }
                        break;
                    }
                }
            }
        } else if (bits & SU_NOTE_SHORT_MASK) {
            // RollHispeed組み込み
            if (noteData->ExtraAttribute->RollHispeedNumber >= 0) {
                if (hispeedDefinitions.find(noteData->ExtraAttribute->RollHispeedNumber) != hispeedDefinitions.end()) {
                    noteData->ExtraAttribute->RollTimeline = hispeedDefinitions[noteData->ExtraAttribute->RollHispeedNumber];
                } else {
                    MakeMessage(0, "Timeline does not exist");
                    noteData->ExtraAttribute->RollHispeedNumber = -1;
                }
            }

            // Air設置処理
            // if ((下に別ノーツがある && それはロング終点) || 下に別ノーツがない)
            if (info.Type[size_t(SusNoteType::Air)] && !info.Type[size_t(SusNoteType::Grounded)]) {
                auto require = true;
                for (const auto &target : notes) {
                    const auto gtime = get<0>(target);
                    const auto &ginfo = get<1>(target);

                    // 判定時刻が異なるノーツは処理対象からはじく
                    if (time != gtime) continue;

                    // 自分自身と位置、サイズが異なるノーツは処理対象からはじく
                    if (info.DefinitionNumber != ginfo.DefinitionNumber) continue;

                    // 自分自身と異なるハイスピ指定がなされているノーツは処理対象からはじく
                    if (info.Timeline != ginfo.Timeline) continue;

                    // ショートノーツならば、Air系でないならば設置する必要なし、Air系なら処理対象からはじく
                    const auto tbits = ginfo.Type.to_ulong();
                    if (tbits & SU_NOTE_SHORT_MASK) {
                        if (ginfo.Type[size_t(SusNoteType::Air)]) continue;

                        require = false;
                        break;
                    }

                    // (ショートノーツでなくて)Slide、Holdでなければ処理対象からはじく
                    if (!ginfo.Type[size_t(SusNoteType::Slide)] && !ginfo.Type[size_t(SusNoteType::Hold)]) continue;

                    // (Slide、Holdの)終点ならば、設置する必要あり
                    if (ginfo.Type[size_t(SusNoteType::End)]) {
                        require = true;
                        break;
                    }

                    // (Slide、Holdの)始点ならば、設置する必要は無いが、確定ではない
                    // 始点終点がかぶっていた場合に定義順(というかソート順)に依存してしまうのでよくわからん
                    if (ginfo.Type[size_t(SusNoteType::Start)]) {
                        require = false;
                        //break;
                    }
                }
                if (require) noteData->Type.set(size_t(SusNoteType::Grounded));
            }

#ifdef SU_ENABLE_NOTE_HORIZONTAL_MOVING
            // 移動レーン処理
            if (SharedMetaData.ExtraFlags[size_t(SusMetaDataFlags::EnableMovingLane)]) {
                for (const auto &startSource : notes) {
                    const auto mltime = get<0>(startSource);
                    const auto &mlinfo = get<1>(startSource);
                    if (mltime != time) continue;
                    if (mlinfo.NotePosition.StartLane != info.NotePosition.StartLane) continue;
                    if (!mlinfo.Type[size_t(SusNoteType::StartPosition)]) continue;
                    noteData->CenterAtZero = mlinfo.Extra + mlinfo.NotePosition.Length / 2.0;
                }
            }
#endif

            SharedMetaData.ScoreDuration = max(SharedMetaData.ScoreDuration, noteData->StartTime);
        } else if (info.Type[size_t(SusNoteType::MeasureLine)]) {
        } else if (!info.Type[size_t(SusNoteType::StartPosition)]) {
            MakeMessage(time.Measure, time.Tick, info.NotePosition.StartLane, "Error with internal representation of notes");
            continue;
        }

        data.push_back(noteData);
    }
}

void SusAnalyzer::CalculateCurves(const shared_ptr<SusDrawableNoteData>& note, NoteCurvesList &curveData) const
{
    auto lastStep = note;
    vector<tuple<double, double>> controlPoints;    // lastStepからの時間, X中央位置(0~1)
    vector<tuple<double, double>> bezierBuffer;

    controlPoints.emplace_back(0, lastStep->CenterAtZero / 16.0);
    for (auto &slideElement : note->ExtraData) {
        if (slideElement->Type.test(size_t(SusNoteType::Injection))) continue;

        const auto cpi = make_tuple(slideElement->StartTime - lastStep->StartTime, slideElement->CenterAtZero / 16.0);
        controlPoints.push_back(cpi);
        if (slideElement->Type.test(size_t(SusNoteType::Control))) continue;

        // EndかStepかInvisible
        const auto segmentPoints = SU_TO_INT32(SharedMetaData.SegmentsPerSecond * (slideElement->StartTime - lastStep->StartTime) + 2);
        vector<tuple<double, double>> segmentPositions;
        for (auto j = 0; j < segmentPoints; j++) {
            const auto relativeTimeInBlock = j / double(segmentPoints - 1);
            bezierBuffer.clear();
            copy(controlPoints.begin(), controlPoints.end(), back_inserter(bezierBuffer));
            for (int k = controlPoints.size() - 1; k >= 0; k--) {
                for (auto l = 0; l < k; l++) {
                    auto derivedTime = glm::mix(get<0>(bezierBuffer[l]), get<0>(bezierBuffer[l + 1]), relativeTimeInBlock);
                    auto derivedPosition = glm::mix(get<1>(bezierBuffer[l]), get<1>(bezierBuffer[l + 1]), relativeTimeInBlock);
                    bezierBuffer[l] = make_tuple(derivedTime, derivedPosition);
                }
            }
            segmentPositions.push_back(bezierBuffer[0]);
        }
        curveData[slideElement] = segmentPositions;
        lastStep = slideElement;
        controlPoints.clear();
        controlPoints.emplace_back(0, slideElement->CenterAtZero / 16.0);
    }
}

uint32_t SusAnalyzer::GetMeasureCount(const uint32_t relativeMeasureCount) const
{
    return measureCountOffset + relativeMeasureCount;
}

uint32_t SusAnalyzer::GetLongNoteChannel(const uint32_t relativeLongNoteChannel) const
{
    return longNoteChannelOffset + relativeLongNoteChannel;
}

// SusHispeedTimeline ------------------------------------------------------------------

const double SusHispeedData::keepSpeed = numeric_limits<double>::quiet_NaN();

SusHispeedTimeline::SusHispeedTimeline(function<double(uint32_t, uint32_t)> func) : relToAbs(std::move(func))
{
    keys.emplace_back(SusRelativeNoteTime { 0, 0 }, SusHispeedData { SusHispeedData::Visibility::Visible, 1.0 });
}

void SusHispeedTimeline::AddKeysByString(const string &def, const function<shared_ptr<SusHispeedTimeline>(uint32_t)>& resolver)
{
    // int'int:double:v/i
    auto str = def;
    vector<string> ks;

    ba::erase_all(str, " ");
    split(ks, str, b::is_any_of(","));
    for (const auto &k : ks) {
        vector<string> params;
        split(params, k, b::is_any_of(":"));
        if (params.size() < 2) {
            //MakeMessage("ハイスピードタイムラインの解析に失敗"); エラーログ出したいが……
            continue;
        }

        if (params[0] == "inherit") {
            // データ流用
            const auto parent = resolver(ConvertHexatridecimal(params[1]));
            if (!parent) continue;

            for (const auto &parentKey : parent->keys) keys.push_back(parentKey);
            continue;
        }

        vector<string> timing;
        split(timing, params[0], b::is_any_of("'"));
        if (timing.size() < 2) {
            //MakeMessage("ハイスピードタイムラインの解析に失敗"); エラーログ出したいが……
            continue;
        }

        const SusRelativeNoteTime time = { ConvertUnsignedInteger(timing[0]), ConvertUnsignedInteger(timing[1]) };
        SusHispeedData data = { SusHispeedData::Visibility::Keep, SusHispeedData::keepSpeed };
        for (auto i = 1u; i < params.size(); i++) {
            if (params[i] == "v" || params[i] == "visible") {
                data.VisibilityState = SusHispeedData::Visibility::Visible;
            } else if (params[i] == "i" || params[i] == "invisible") {
                data.VisibilityState = SusHispeedData::Visibility::Invisible;
            } else {
                data.Speed = ConvertFloat(params[i]);
            }
        }

        auto found = false;
        for (auto &p : keys) {
            if (p.first == time) {
                p.second = data;
                found = true;
                break;
            }
        }
        if (!found) keys.emplace_back(time, data);
    }
}

void SusHispeedTimeline::AddKeyByData(const uint32_t meas, const uint32_t tick, const double hs)
{
    SusRelativeNoteTime time = { meas, tick };
    for (auto &p : keys) {
        if (p.first != time) continue;
        p.second.Speed = hs;
        return;
    }
    SusHispeedData data = { SusHispeedData::Visibility::Keep, hs };
    keys.emplace_back(time, data);
}

void SusHispeedTimeline::AddKeyByData(const uint32_t meas, const uint32_t tick, const bool vis)
{
    const SusRelativeNoteTime time = { meas, tick };
    const auto vv = vis ? SusHispeedData::Visibility::Visible : SusHispeedData::Visibility::Invisible;
    for (auto &p : keys) {
        if (p.first != time) continue;
        p.second.VisibilityState = vv;
        return;
    }
    SusHispeedData data = { vv, SusHispeedData::keepSpeed };
    keys.emplace_back(time, data);
}

void SusHispeedTimeline::Finialize()
{
    stable_sort(keys.begin(), keys.end(), [](const pair<SusRelativeNoteTime, SusHispeedData> &a, const pair<SusRelativeNoteTime, SusHispeedData> &b) {
        return a.first < b.first;
    });

    auto hs = 1.0;
    auto vis = true;
    for (auto &key : keys) {
        if (!isnan(key.second.Speed)) {
            hs = key.second.Speed;
        } else {
            key.second.Speed = hs;
        }
        if (key.second.VisibilityState != SusHispeedData::Visibility::Keep) {
            vis = key.second.VisibilityState == SusHispeedData::Visibility::Visible;
        } else {
            key.second.VisibilityState = vis ? SusHispeedData::Visibility::Visible : SusHispeedData::Visibility::Invisible;
        }
    }

    double sum = 0;
    double lastAt = 0;
    auto lastSpeed = 1.0;
    for (const auto &rd : keys) {
        const auto t = relToAbs(rd.first.Measure, rd.first.Tick);
        sum += (t - lastAt) * lastSpeed;
        data.emplace_back(t, sum, rd.second);
        lastAt = t;
        lastSpeed = rd.second.Speed;
    }
    keys.clear();
}

tuple<bool, double> SusHispeedTimeline::GetRawDrawStateAt(const double time)
{
    auto lastData = data[0];
    auto check = 0;
    for (const auto &d : data) {
        if (!check++) continue;
        const auto keyTime = get<0>(d);
        if (keyTime >= time) break;
        lastData = d;
    }
    const auto lastDifference = time - get<0>(lastData);
    return make_tuple(get<2>(lastData).VisibilityState == SusHispeedData::Visibility::Visible, get<1>(lastData) + lastDifference * get<2>(lastData).Speed);
}

double SusHispeedTimeline::GetSpeedAt(const double time)
{
    auto lastData = data[0];
    auto check = 0;
    for (const auto &d : data) {
        if (!check++) continue;
        const auto keyTime = get<0>(d);
        if (keyTime >= time) break;
        lastData = d;
    }
    return get<2>(lastData).Speed;
}

tuple<bool, double> SusDrawableNoteData::GetStateAt(const double time)
{
    auto result = Timeline->GetRawDrawStateAt(time);
    ModifiedPosition = StartTimeEx - get<1>(result);
    for (auto &ex : ExtraData) {
        if (!ex->Timeline) {
            ex->ModifiedPosition = numeric_limits<double>::quiet_NaN();
            continue;
        }
        auto eres = ex->Timeline->GetRawDrawStateAt(time);
        ex->ModifiedPosition = ex->StartTimeEx - get<1>(eres);
    }
    return make_tuple(get<0>(result), StartTimeEx - get<1>(result));
}

void SusNoteExtraAttribute::Apply(const string &props)
{
    using namespace boost::algorithm;
    auto list = props;
    list.erase(remove(list.begin(), list.end(), ' '), list.end());
    vector<string> params;
    split(params, list, is_any_of(","));

    vector<string> pr;
    for (const auto& p : params) {
        pr.clear();
        split(pr, p, is_any_of(":"));
        if (pr.size() != 2) continue;
        switch (Crc32Rec(0xffffffff, pr[0].c_str())) {
            case "priority"_crc32:
            case "pr"_crc32:
                Priority = uint32_t(strtol(pr[1].c_str(), nullptr, 10));
                break;
            case "rollhs"_crc32:
            case "rh"_crc32:
                RollHispeedNumber = ConvertHexatridecimal(pr[1]);
                break;
            case "height"_crc32:
            case "h"_crc32:
                HeightScale = double(strtod(pr[1].c_str(), nullptr));
                break;
            default:
                // TODO: メッセージ出す
                break;
        }
    }
}
