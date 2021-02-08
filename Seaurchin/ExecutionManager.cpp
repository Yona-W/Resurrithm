#include "ExecutionManager.h"

#include "Config.h"
#include "Setting.h"
#include "Interfaces.h"

#include "ScriptResource.h"
#include "ScriptScene.h"
#include "ScriptSprite.h"
#include "MoverFunctionExpression.h"
#include "ScenePlayer.h"
#include "CharacterInstance.h"

using namespace boost::filesystem;
using namespace std;
using namespace Rendering;

#ifdef _WIN32

const static toml::Array defaultSliderKeys = {
    toml::Array { KEY_INPUT_A }, toml::Array { KEY_INPUT_Z }, toml::Array { KEY_INPUT_S }, toml::Array { KEY_INPUT_X },
    toml::Array { KEY_INPUT_D }, toml::Array { KEY_INPUT_C }, toml::Array { KEY_INPUT_F }, toml::Array { KEY_INPUT_V },
    toml::Array { KEY_INPUT_G }, toml::Array { KEY_INPUT_B }, toml::Array { KEY_INPUT_H }, toml::Array { KEY_INPUT_N },
    toml::Array { KEY_INPUT_J }, toml::Array { KEY_INPUT_M }, toml::Array { KEY_INPUT_K }, toml::Array { KEY_INPUT_COMMA }
};

const static toml::Array defaultAirStringKeys = {
    KEY_SLASH, KEY_DOT, KEY_APOSTROPHE, KEY_SEMICOLON, KEY_LEFTBRACE, KEY_RIGHTBRACE // TODO fix for windows
};

#else

const static toml::Array defaultSliderKeys = {
    toml::Array { SDL_SCANCODE_A, SDL_SCANCODE_1 }, toml::Array { SDL_SCANCODE_Z, SDL_SCANCODE_Q }, toml::Array { SDL_SCANCODE_S, SDL_SCANCODE_2 }, toml::Array { SDL_SCANCODE_X, SDL_SCANCODE_W },
    toml::Array { SDL_SCANCODE_D, SDL_SCANCODE_3 }, toml::Array { SDL_SCANCODE_C, SDL_SCANCODE_E }, toml::Array { SDL_SCANCODE_F, SDL_SCANCODE_4 }, toml::Array { SDL_SCANCODE_V, SDL_SCANCODE_R },
    toml::Array { SDL_SCANCODE_G, SDL_SCANCODE_5 }, toml::Array { SDL_SCANCODE_B, SDL_SCANCODE_T }, toml::Array { SDL_SCANCODE_H, SDL_SCANCODE_6 }, toml::Array { SDL_SCANCODE_N, SDL_SCANCODE_Y },
    toml::Array { SDL_SCANCODE_J, SDL_SCANCODE_7 }, toml::Array { SDL_SCANCODE_M, SDL_SCANCODE_U }, toml::Array { SDL_SCANCODE_K, SDL_SCANCODE_8 }, toml::Array { SDL_SCANCODE_COMMA, SDL_SCANCODE_I }
};

const static toml::Array defaultAirStringKeys = {
    SDL_SCANCODE_SLASH, SDL_SCANCODE_PERIOD, SDL_SCANCODE_APOSTROPHE, SDL_SCANCODE_SEMICOLON, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET
};

#endif

ExecutionManager::ExecutionManager(const shared_ptr<Setting>& setting)
    : sharedSetting(setting)
    , settingManager(new setting2::SettingItemManager(sharedSetting))
    , scriptInterface(new AngelScript())
    , sound(new SoundManager())
    , musics(new MusicsManager(this)) // this渡すの怖いけどMusicsManagerのコンストラクタ内で逆参照してないから多分セーフ
    , characters(new CharacterManager())
    , skills(new SkillManager())
    //, extensions(new ExtensionManager())
    , random(new mt19937(random_device()()))
    , sharedControlState(new ControlState)
    , lastResult()
    #ifdef _WIN32
    , hImc(nullptr)
    , hCommunicationPipe(nullptr)
    , immConversion(0)
    , immSentence(0)
    #endif
    , mixerBgm(nullptr)
    , mixerSe(nullptr)
{}

void ExecutionManager::Initialize()
{
    shouldExit = false;
    
    std::ifstream slfile;
    string procline;
    // ルートのSettingList読み込み
    const auto slpath = sharedSetting->GetRootDirectory() / SU_DATA_DIR / SU_SCRIPT_DIR / SU_SETTING_DEFINITION_FILE;
    settingManager->LoadItemsFromToml(slpath);
    settingManager->RetrieveAllValues();

    // 入力設定
    sharedControlState->Initialize();

    auto loadedSliderKeys = sharedSetting->ReadValue<toml::Array>("Play", "SliderKeys", defaultSliderKeys);
    if (loadedSliderKeys.size() >= 16) {
        for (auto i = 0; i < 16; i++) sharedControlState->SetSliderKeybindings(i, loadedSliderKeys[i].as<vector<int>>());
    } else {
        spdlog::warn("Configuration contains less than 16 slider key sets, using default.");
    }

    auto loadedAirKeys = sharedSetting->ReadValue<toml::Array>("Play", "AirStringKeys", defaultAirStringKeys);
    if (loadedAirKeys.size() > 0) {
        auto keys = std::vector<int>();
        for (auto key : loadedAirKeys) keys.push_back(key.as<int>());
        sharedControlState->SetAirKeybindings(keys);
    } else {
        spdlog::warn(u8"No air keys defined, using default");
    }

    // 拡張ライブラリ読み込み
    /*
    extensions->LoadExtensions();
    extensions->Initialize(scriptInterface->GetEngine());
    */

    // サウンド初期化
    mixerBgm = SSoundMixer::CreateMixer(sound.get());
    mixerSe = SSoundMixer::CreateMixer(sound.get());

    // AngelScriptインターフェース登録
    InterfacesRegisterEnum(this);
    RegisterScriptResource(this);
    RegisterScriptSprite(this);
    RegisterScriptScene(this);
    RegisterScriptSkin(this);
    RegisterCharacterSkillTypes(scriptInterface->GetEngine());
    RegisterPlayerScene(this);
    InterfacesRegisterSceneFunction(this);
    InterfacesRegisterGlobalFunction(this);
    RegisterGlobalManagementFunction();
    //extensions->RegisterInterfaces();

    // キャラ・スキル読み込み
    characters->LoadAllCharacters();
    skills->LoadAllSkills();

    # ifdef _WIN32
    // 外部通信
    hCommunicationPipe = CreateNamedPipe(
        SU_NAMED_PIPE_NAME,
        PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        3, 0, 0, 1000,
        nullptr
    );
    #endif
    /*
    hImc = ImmGetContext(GetMainWindowHandle());
    if (!ImmGetOpenStatus(hImc)) ImmSetOpenStatus(hImc, TRUE);
    ImmGetConversionStatus(hImc, &ImmConversion, &ImmSentence);
    ImmSetConversionStatus(hImc, IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE, ImmSentence);
    */
}

void ExecutionManager::ExitApplication(){
    shouldExit = true;
}

void ExecutionManager::Shutdown()
{
    for (auto& scene : scenes) scene->Disappear();
    scenes.clear();
    for (auto& scene : scenesPending) scene->Disappear();
    scenesPending.clear();

    if (skin) skin->Terminate();
    settingManager->SaveAllValues();
    sharedControlState->Terminate();

    BOOST_ASSERT(mixerBgm->GetRefCount() == 1);
    BOOST_ASSERT(mixerSe->GetRefCount() == 1);

    mixerBgm->Release();
    mixerSe->Release();

    # ifdef _WIN32
    if (hCommunicationPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hCommunicationPipe);
        CloseHandle(hCommunicationPipe);
    }
    #endif
}

void ExecutionManager::RegisterGlobalManagementFunction()
{
    auto engine = scriptInterface->GetEngine();
    MusicSelectionCursor::RegisterScriptInterface(engine);

    engine->RegisterGlobalFunction("void ExitApplication()", asMETHOD(ExecutionManager, ExitApplication), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("void WriteLog(const string &in)", asMETHOD(ExecutionManager, WriteLog), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("void Fire(const string &in)", asMETHOD(ExecutionManager, Fire), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction(SU_IF_SETTING_ITEM "@ GetSettingItem(const string &in, const string &in)", asMETHOD(ExecutionManager, GetSettingItem), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("bool ExistsData(const string &in)", asMETHOD(ExecutionManager, ExistsData), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("void SetData(const string &in, const bool &in)", asMETHOD(ExecutionManager, SetBoolData), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("void SetData(const string &in, const int &in)", asMETHOD(ExecutionManager, SetIntData), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("void SetData(const string &in, const double &in)", asMETHOD(ExecutionManager, SetDoubleData), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("void SetData(const string &in, const string &in)", asMETHOD(ExecutionManager, SetStringData), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("bool GetBoolData(const string &in)", asMETHODPR(ExecutionManager, GetData<bool>, (const string&), bool), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("int GetIntData(const string &in)", asMETHODPR(ExecutionManager, GetData<int>, (const string&), int), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("double GetDoubleData(const string &in)", asMETHODPR(ExecutionManager, GetData<double>, (const string&), double), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("string GetStringData(const string &in)", asMETHODPR(ExecutionManager, GetData<string>, (const string&), string), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("bool RegisterMoverFunction(const string &in, const string &in)", asFUNCTIONPR(MoverFunctionExpressionManager::Register, (const string&, const string&), bool), asCALL_CDECL);
    engine->RegisterGlobalFunction("bool IsMoverFunctionRegistered(const string &in)", asFUNCTIONPR(MoverFunctionExpressionManager::IsRegistered, (const string&), bool), asCALL_CDECL);

    engine->RegisterGlobalFunction(SU_IF_CHARACTER_MANAGER "@ GetCharacterManager()", asMETHOD(ExecutionManager, GetCharacterManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction(SU_IF_SKILL_MANAGER "@ GetSkillManager()", asMETHOD(ExecutionManager, GetSkillManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("bool Execute(const string &in)", asMETHODPR(ExecutionManager, ExecuteSkin, (const string&), bool), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("bool ExecuteScene(" SU_IF_SCENE "@)", asMETHODPR(ExecutionManager, ExecuteScene, (asIScriptObject*), bool), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction("bool ExecuteScene(" SU_IF_COSCENE "@)", asMETHODPR(ExecutionManager, ExecuteScene, (asIScriptObject*), bool), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterGlobalFunction(SU_IF_SOUNDMIXER "@ GetDefaultMixer(const string &in)", asMETHOD(ExecutionManager, GetDefaultMixer), asCALL_THISCALL_ASGLOBAL, this);
    engine->RegisterObjectBehaviour(SU_IF_MSCURSOR, asBEHAVE_FACTORY, SU_IF_MSCURSOR "@ f()", asMETHOD(MusicsManager, CreateMusicSelectionCursor), asCALL_THISCALL_ASGLOBAL, musics.get());
    engine->RegisterObjectBehaviour(SU_IF_SCENE_PLAYER, asBEHAVE_FACTORY, SU_IF_SCENE_PLAYER "@ f()", asMETHOD(ExecutionManager, CreatePlayer), asCALL_THISCALL_ASGLOBAL, this);
}


void ExecutionManager::EnumerateSkins()
{
    using namespace boost;
    using namespace boost::filesystem;
    using namespace xpressive;
    

    const auto sepath = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR;

    for (const auto& fdata : make_iterator_range(directory_iterator(sepath), {})) {
        if (!is_directory(fdata)) continue;
        if (!CheckSkinStructure(fdata.path())) continue;
        skinNames.push_back(fdata.path().filename().string());
    }
    spdlog::info(u8"Number of skins: {0:d}", skinNames.size());
}

bool ExecutionManager::CheckSkinStructure(const path& name) const
{
    using namespace boost;
    using namespace boost::filesystem;

    if (!exists(name / SU_SKIN_MAIN_FILE)) return false;
    if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_TITLE_FILE)) return false;
    if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_SELECT_FILE)) return false;
    if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_PLAY_FILE)) return false;
    if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_RESULT_FILE)) return false;
    return true;
}

void ExecutionManager::ExecuteSkin()
{
    using namespace boost::filesystem;
    

    const auto sn = sharedSetting->ReadValue<string>(SU_SETTING_GENERAL, SU_SETTING_SKIN, "Default");
    if (find(skinNames.begin(), skinNames.end(), sn) == skinNames.end()) {
        spdlog::error("Skin  \"{0}\" not found!", sn);
        return;
    }
    const auto skincfg = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / sn / SU_SETTING_DEFINITION_FILE;
    if (exists(skincfg)) {
        spdlog::info("Skin configuration found");
        settingManager->LoadItemsFromToml(skincfg);
        settingManager->RetrieveAllValues();
    }

    skin = make_unique<SkinHolder>(sn, scriptInterface, sound);
    skin->Initialize();
    spdlog::info("Skin loaded");
    ExecuteSkin(SU_SKIN_TITLE_FILE);
}

bool ExecutionManager::ExecuteSkin(const string &file)
{
    
    const auto obj = skin->ExecuteSkinScript(file);
    if (!obj) {
        spdlog::error("Skin script compile failed");
        return false;
    }
    const auto s = CreateSceneFromScriptObject(obj);
    if (!s) {
        spdlog::error("No EntryPoint in {0}", file);
        obj->Release();
        return false;
    }

    spdlog::info("Executing scene from skin file: {0}", file);
    AddScene(s);

    obj->Release();
    return true;
}

bool ExecutionManager::ExecuteScene(asIScriptObject *sceneObject)
{
    
    const auto s = CreateSceneFromScriptObject(sceneObject);
    if (!s) return false;
    sceneObject->SetUserData(skin.get(), SU_UDTYPE_SKIN);
    AddScene(s);

    sceneObject->Release();
    return true;
}

void ExecutionManager::ExecuteSystemMenu()
{
    using namespace boost;
    using namespace boost::filesystem;
    

    auto sysmf = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SCRIPT_DIR / SU_SYSTEM_MENU_FILE;
    if (!exists(sysmf)) {
        spdlog::error("No system menu script");
        return;
    }

    scriptInterface->StartBuildModule("SystemMenu", [](auto inc, auto from, auto sb) { return true; });
    scriptInterface->LoadFile(sysmf.string());
    if (!scriptInterface->FinishBuildModule()) {
        spdlog::error("System menu script failed to compile");
        return;
    }
    const auto mod = scriptInterface->GetLastModule();

    //エントリポイント検索
    const int cnt = mod->GetObjectTypeCount();
    asITypeInfo *type = nullptr;
    for (auto i = 0; i < cnt; i++) {
        const auto cti = mod->GetObjectTypeByIndex(i);
        if (!scriptInterface->CheckMetaData(cti, "EntryPoint")) continue;
        type = cti;
        type->AddRef();
        break;
    }
    if (!type) {
        spdlog::error(u8"No EntryPoint in system menu script");
        return;
    }

    spdlog::info("Loading system menu from script");
    AddScene(CreateSceneFromScriptType(type));

    type->Release();
}


//Tick
void ExecutionManager::Tick(const double delta)
{
    if(!sharedControlState->Update()){
        shouldExit = true;
    }

    //シーン操作
    for (auto& scene : scenesPending) scenes.push_back(scene);
    scenesPending.clear();
    sort(scenes.begin(), scenes.end(), [](const shared_ptr<Scene> sa, const shared_ptr<Scene> sb) { return sa->GetIndex() < sb->GetIndex(); });
    auto i = scenes.begin();
    while (i != scenes.end()) {
        (*i)->Tick(delta);
        if ((*i)->IsDead()) {
            (*i)->Dispose();
            i = scenes.erase(i);
        } else {
            ++i;
        }
    }

    //後処理
    static double ps = 0;
    ps += delta;
    if (ps >= 1.0) {
        ps = 0;
        mixerBgm->Update();
        mixerSe->Update();
    }
    scriptInterface->GetEngine()->GarbageCollect(asGC_ONE_STEP);
}

void ExecutionManager::Draw()
{
    //
    //spdlog::info("Drawing frame");

    GPU_Clear(gpu);
    for (const auto& s : scenes) s->Draw();
    GPU_Flip(gpu);
}

void ExecutionManager::AddScene(const shared_ptr<Scene>& scene)
{
    scenesPending.push_back(scene);
    scene->SetManager(this);
    scene->Initialize();
    spdlog::info("Added scene");
}

shared_ptr<ScriptScene> ExecutionManager::CreateSceneFromScriptType(asITypeInfo *type) const
{
    
    shared_ptr<ScriptScene> ret;
    if (scriptInterface->CheckImplementation(type, SU_IF_COSCENE)) {
        auto obj = scriptInterface->InstantiateObject(type);
        return static_pointer_cast<ScriptScene>(make_shared<ScriptCoroutineScene>(obj));
    }
    if (scriptInterface->CheckImplementation(type, SU_IF_SCENE))  //最後
    {
        auto obj = scriptInterface->InstantiateObject(type);
        return make_shared<ScriptScene>(obj);
    }
    spdlog::error(u8"{0} does not implement the Scene interface", type->GetName());
    return nullptr;
}

shared_ptr<ScriptScene> ExecutionManager::CreateSceneFromScriptObject(asIScriptObject *obj) const
{
    
    shared_ptr<ScriptScene> ret;
    const auto type = obj->GetObjectType();
    if (scriptInterface->CheckImplementation(type, SU_IF_COSCENE)) {
        return static_pointer_cast<ScriptScene>(make_shared<ScriptCoroutineScene>(obj));
    }
    if (scriptInterface->CheckImplementation(type, SU_IF_SCENE))  //最後
    {
        return make_shared<ScriptScene>(obj);
    }
    spdlog::error(u8"{0} does not implement the Scene interface", type->GetName());
    return nullptr;
}