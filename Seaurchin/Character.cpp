#include "Character.h"
#include "ExecutionManager.h"
#include "Misc.h"
#include "Setting.h"
#include "Config.h"

using namespace std;

CharacterManager::CharacterManager()
{
    selected = -1;
}

void CharacterManager::LoadAllCharacters()
{
    using namespace boost;
    using namespace boost::filesystem;
    using namespace xpressive;
    

    const auto sepath = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_CHARACTER_DIR;

    for (const auto& fdata : make_iterator_range(directory_iterator(sepath), {})) {
        if (is_directory(fdata)) continue;
        const auto filename = fdata.path().string();
        if (!ends_with(filename, ".toml")) continue;
        LoadFromToml(fdata.path());
    }
    spdlog::info("Characters loaded: {0:d}", characters.size());
    selected = 0;
}

void CharacterManager::Next()
{
    selected = (selected + characters.size() + 1) % characters.size();
}

void CharacterManager::Previous()
{
    selected = (selected + characters.size() - 1) % characters.size();
}

CharacterParameter* CharacterManager::GetCharacterParameter(const int relative)
{
    auto ri = selected + relative;
    while (ri < 0) ri += characters.size();
    return characters[ri % characters.size()].get();
}

shared_ptr<CharacterParameter> CharacterManager::GetCharacterParameterSafe(const int relative)
{
    auto ri = selected + relative;
    while (ri < 0) ri += characters.size();
    return characters[ri % characters.size()];
}

CharacterImageSet* CharacterManager::CreateCharacterImages(const int relative)
{
    auto ri = selected + relative;
    while (ri < 0) ri += characters.size();
    const auto cp = characters[ri % characters.size()];
    return CharacterImageSet::CreateImageSet(cp);
}

int32_t CharacterManager::GetSize() const
{
    return int32_t(characters.size());
}


void CharacterManager::LoadFromToml(const boost::filesystem::path& file)
{
    using namespace boost::filesystem;

    
    auto result = make_shared<CharacterParameter>();

    std::ifstream ifs(file.string(), ios::in);
    auto pr = toml::parse(ifs);
    ifs.close();
    if (!pr.valid()) {
        spdlog::error(u8"Failed to load {0}: invalid file", file.string());
        spdlog::error(pr.errorReason);
        return;
    }
    auto &root = pr.value;

    try {
        result->Name = root.get<string>("Name");
        auto imgpath = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_CHARACTER_DIR / root.get<string>("Image");
        result->ImagePath = imgpath.string();

        const auto ws = root.find("Metric.WholeScale");
        result->Metric.WholeScale = (ws && ws->is<double>()) ? ws->as<double>() : 1.0;

        const auto fo = root.find("Metric.Face");
        if (fo && fo->is<vector<int>>()) {
            auto arr = fo->as<vector<int>>();
            for (auto i = 0; i < 2; i++) result->Metric.FaceOrigin[i] = arr[i];
        } else {
            result->Metric.FaceOrigin[0] = 0;
            result->Metric.FaceOrigin[1] = 0;
        }

        const auto sr = root.find("Metric.SmallRange");
        if (sr && sr->is<vector<int>>()) {
            auto arr = sr->as<vector<int>>();
            for (auto i = 0; i < 4; i++) result->Metric.SmallRange[i] = arr[i];
        } else {
            result->Metric.SmallRange[0] = 0;
            result->Metric.SmallRange[1] = 0;
            result->Metric.SmallRange[2] = 280;
            result->Metric.SmallRange[3] = 170;
        }

        const auto fr = root.find("Metric.FaceRange");
        if (fr && fr->is<vector<int>>()) {
            auto arr = fr->as<vector<int>>();
            for (auto i = 0; i < 4; i++) result->Metric.FaceRange[i] = arr[i];
        } else {
            result->Metric.FaceRange[0] = 0;
            result->Metric.FaceRange[1] = 0;
            result->Metric.FaceRange[2] = 128;
            result->Metric.FaceRange[3] = 128;
        }
    } catch (exception ex) {
        spdlog::error(u8"Failed to load character {0} - unknown error", file.string());
        return;
    }
    spdlog::info("Character loaded: {0}", result->Name);
    characters.push_back(result);
}


CharacterImageSet::CharacterImageSet(const shared_ptr<CharacterParameter> param)
{
    parameter = param;
    LoadAllImage();
}

CharacterImageSet::~CharacterImageSet()
{
    imageFull->Release();
    imageSmall->Release();
    imageFace->Release();
}

void CharacterImageSet::ApplyFullImage(SSprite *sprite) const
{
    const auto cx = parameter->Metric.FaceOrigin[0];
    const auto cy = parameter->Metric.FaceOrigin[1];
    const auto sc = parameter->Metric.WholeScale;
    ostringstream ss;
    ss << "origX:" << cx << ", origY:" << cy << ", scaleX:" << sc << ", scaleY:" << sc;

    imageFull->AddRef();
    sprite->SetImage(imageFull);
    sprite->Apply(ss.str());
    sprite->Release();
}

void CharacterImageSet::ApplySmallImage(SSprite *sprite) const
{
    imageSmall->AddRef();
    sprite->SetImage(imageSmall);
    sprite->Release();
}

void CharacterImageSet::ApplyFaceImage(SSprite *sprite) const
{
    imageFace->AddRef();
    sprite->SetImage(imageFace);
    sprite->Release();
}

CharacterImageSet *CharacterImageSet::CreateImageSet(const shared_ptr<CharacterParameter> param)
{
    auto result = new CharacterImageSet(param);
    result->AddRef();
    return result;
}

void CharacterImageSet::LoadAllImage()
{
    auto root = parameter->ImagePath;
    const auto baseSurface = IMG_Load(reinterpret_cast<const char*>(root.c_str()));
    const auto baseTex = GPU_CopyImageFromSurface(baseSurface);
    const auto smallTex = GPU_CreateImage(SU_CHAR_SMALL_WIDTH, SU_CHAR_SMALL_WIDTH, GPU_FormatEnum::GPU_FORMAT_RGBA);
    const auto faceTex = GPU_CreateImage(SU_CHAR_FACE_SIZE, SU_CHAR_FACE_SIZE, GPU_FormatEnum::GPU_FORMAT_RGBA);
    const auto smallTarget = GPU_LoadTarget(smallTex);
    const auto faceTarget = GPU_LoadTarget(faceTex);

    GPU_Rect smallRectSrc = {
        parameter->Metric.SmallRange[0], parameter->Metric.SmallRange[1],
        parameter->Metric.SmallRange[2], parameter->Metric.SmallRange[3]
    };

    GPU_Rect faceRectSrc = {
        parameter->Metric.FaceRange[0], parameter->Metric.FaceRange[1],
        parameter->Metric.FaceRange[2], parameter->Metric.FaceRange[3],
    };

    GPU_BlitRect(baseTex, &smallRectSrc, smallTarget, NULL);
    GPU_BlitRect(baseTex, &faceRectSrc, faceTarget, NULL);

    GPU_FreeTarget(smallTarget);
    GPU_FreeTarget(faceTarget);

    imageFull = new SImage(baseTex);
    imageFull->AddRef();
    imageSmall = new SImage(smallTex);
    imageSmall->AddRef();
    imageFace = new SImage(faceTex);
    imageFace->AddRef();
}

void CharacterImageSet::RegisterType(asIScriptEngine *engine)
{
    engine->RegisterObjectType(SU_IF_CHARACTER_IMAGES, 0, asOBJ_REF);
    engine->RegisterObjectBehaviour(SU_IF_CHARACTER_IMAGES, asBEHAVE_ADDREF, "void f()", asMETHOD(CharacterImageSet, AddRef), asCALL_THISCALL);
    engine->RegisterObjectBehaviour(SU_IF_CHARACTER_IMAGES, asBEHAVE_RELEASE, "void f()", asMETHOD(CharacterImageSet, Release), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_IMAGES, "void ApplyFullImage(" SU_IF_SPRITE "@)", asMETHOD(CharacterImageSet, ApplyFullImage), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_IMAGES, "void ApplySmallImage(" SU_IF_SPRITE "@)", asMETHOD(CharacterImageSet, ApplySmallImage), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_IMAGES, "void ApplyFaceImage(" SU_IF_SPRITE "@)", asMETHOD(CharacterImageSet, ApplyFaceImage), asCALL_THISCALL);
}

void RegisterCharacterTypes(asIScriptEngine *engine)
{
    CharacterImageSet::RegisterType(engine);

    engine->RegisterObjectType(SU_IF_CHARACTER_METRIC, sizeof(CharacterImageMetric), asOBJ_VALUE | asOBJ_POD);
    engine->RegisterObjectProperty(SU_IF_CHARACTER_METRIC, "double WholeScale", asOFFSET(CharacterImageMetric, WholeScale));
    engine->RegisterObjectMethod(SU_IF_CHARACTER_METRIC, "int get_FaceOrigin(uint)", asMETHOD(CharacterImageMetric, GetFaceOrigin), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_METRIC, "int get_SmalLRange(uint)", asMETHOD(CharacterImageMetric, GetSmallRange), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_METRIC, "int get_FaceRange(uint)", asMETHOD(CharacterImageMetric, GetFaceRange), asCALL_THISCALL);

    engine->RegisterObjectType(SU_IF_CHARACTER_PARAM, 0, asOBJ_REF | asOBJ_NOCOUNT);
    engine->RegisterObjectProperty(SU_IF_CHARACTER_PARAM, "string Name", asOFFSET(CharacterParameter, Name));
    engine->RegisterObjectProperty(SU_IF_CHARACTER_PARAM, "string ImagePath", asOFFSET(CharacterParameter, ImagePath));
    engine->RegisterObjectProperty(SU_IF_CHARACTER_PARAM, SU_IF_CHARACTER_METRIC " Metric", asOFFSET(CharacterParameter, Metric));

    engine->RegisterObjectType(SU_IF_CHARACTER_MANAGER, 0, asOBJ_REF | asOBJ_NOCOUNT);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, "void Next()", asMETHOD(CharacterManager, Next), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, "void Previous()", asMETHOD(CharacterManager, Previous), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, "int GetSize()", asMETHOD(CharacterManager, GetSize), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, SU_IF_CHARACTER_PARAM "@ GetCharacter(int)", asMETHOD(CharacterManager, GetCharacterParameter), asCALL_THISCALL);
    engine->RegisterObjectMethod(SU_IF_CHARACTER_MANAGER, SU_IF_CHARACTER_IMAGES "@ CreateCharacterImages(int)", asMETHOD(CharacterManager, CreateCharacterImages), asCALL_THISCALL);
}
