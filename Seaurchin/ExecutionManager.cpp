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

using namespace std::filesystem;
using namespace std;

const static toml::Array defaultSliderKeys = {
	toml::Array { KEY_INPUT_A }, toml::Array { KEY_INPUT_Z }, toml::Array { KEY_INPUT_S }, toml::Array { KEY_INPUT_X },
	toml::Array { KEY_INPUT_D }, toml::Array { KEY_INPUT_C }, toml::Array { KEY_INPUT_F }, toml::Array { KEY_INPUT_V },
	toml::Array { KEY_INPUT_G }, toml::Array { KEY_INPUT_B }, toml::Array { KEY_INPUT_H }, toml::Array { KEY_INPUT_N },
	toml::Array { KEY_INPUT_J }, toml::Array { KEY_INPUT_M }, toml::Array { KEY_INPUT_K }, toml::Array { KEY_INPUT_COMMA }
};

const static toml::Array defaultAirStringKeys = {
	toml::Array { KEY_INPUT_PGUP }, toml::Array { KEY_INPUT_PGDN }, toml::Array { KEY_INPUT_HOME }, toml::Array { KEY_INPUT_END }
};

ExecutionManager::ExecutionManager(const shared_ptr<Setting>& setting)
	: sharedSetting(setting)
	, settingManager(new setting2::SettingItemManager(sharedSetting))
	, scriptInterface(new AngelScript())
	, sound(new SoundManager())
	, musics(new MusicsManager())
	, characters(new CharacterManager())
	, skills(new SkillManager())
	, extensions(new ExtensionManager())
	, random(new mt19937(random_device()()))
	, sharedControlState(new ControlState)
	, lastResult()
	, hImc(nullptr)
	, hCommunicationPipe(nullptr)
	, immConversion(0)
	, immSentence(0)
	, mixerBgm(nullptr)
	, mixerSe(nullptr)
{}

void ExecutionManager::Initialize()
{
	auto log = spdlog::get("main");
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
		for (auto i = 0; i < 16; i++) sharedControlState->SetSliderKeyCombination(i, loadedSliderKeys[i].as<vector<int>>());
	}
	else {
		log->warn(u8"スライダーキー設定の配列が16要素未満のため、フォールバックを利用します");
	}

	auto loadedAirStringKeys = sharedSetting->ReadValue<toml::Array>("Play", "AirStringKeys", defaultAirStringKeys);
	if (loadedAirStringKeys.size() >= 4) {
		for (auto i = 0; i < 4; i++) sharedControlState->SetAirStringKeyCombination(i, loadedAirStringKeys[i].as<vector<int>>());
	}
	else {
		log->warn(u8"エアストリングキー設定の配列が4要素未満のため、フォールバックを利用します");
	}

	// 拡張ライブラリ読み込み
	extensions->LoadExtensions();
	extensions->Initialize(scriptInterface->GetEngine());

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
	extensions->RegisterInterfaces();

	// キャラ・スキル読み込み
	characters->LoadAllCharacters();
	skills->LoadAllSkills();

	// 外部通信
	hCommunicationPipe = CreateNamedPipe(
		SU_NAMED_PIPE_NAME,
		PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		3, 0, 0, 1000,
		nullptr
	);

	/*
	hImc = ImmGetContext(GetMainWindowHandle());
	if (!ImmGetOpenStatus(hImc)) ImmSetOpenStatus(hImc, TRUE);
	ImmGetConversionStatus(hImc, &ImmConversion, &ImmSentence);
	ImmSetConversionStatus(hImc, IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE, ImmSentence);
	*/
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
	if (hCommunicationPipe != INVALID_HANDLE_VALUE) {
		DisconnectNamedPipe(hCommunicationPipe);
		CloseHandle(hCommunicationPipe);
	}
}

void ExecutionManager::RegisterGlobalManagementFunction()
{
	auto engine = scriptInterface->GetEngine();
	MusicsManager::RegisterScriptInterface(engine);

	engine->RegisterGlobalFunction("void ExitApplication()", asFUNCTION(InterfacesExitApplication), asCALL_CDECL);
	engine->RegisterGlobalFunction("void WriteLog(const string &in)", asMETHOD(ExecutionManager, WriteLog), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void Fire(const string &in)", asMETHOD(ExecutionManager, Fire), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction(SU_IF_SETTING_ITEM "@ GetSettingItem(const string &in, const string &in)", asMETHOD(ExecutionManager, GetSettingItem), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool ExistsData(const string &in)", asMETHOD(ExecutionManager, ExistsData), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void SetData(const string &in, const bool &in)", asMETHOD(ExecutionManager, SetData<bool>), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void SetData(const string &in, const int &in)", asMETHOD(ExecutionManager, SetData<int>), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void SetData(const string &in, const double &in)", asMETHOD(ExecutionManager, SetData<double>), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void SetData(const string &in, const string &in)", asMETHOD(ExecutionManager, SetData<string>), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool GetBoolData(const string &in)", asMETHODPR(ExecutionManager, GetData<bool>, (const string&), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("int GetIntData(const string &in)", asMETHODPR(ExecutionManager, GetData<int>, (const string&), int), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("double GetDoubleData(const string &in)", asMETHODPR(ExecutionManager, GetData<double>, (const string&), double), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("string GetStringData(const string &in)", asMETHODPR(ExecutionManager, GetData<string>, (const string&), string), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool RegisterMoverFunction(const string &in, const string &in)", asFUNCTIONPR(MoverFunctionExpressionManager::Register, (const string&, const string&), bool), asCALL_CDECL);
	engine->RegisterGlobalFunction("bool IsMoverFunctionRegistered(const string &in)", asFUNCTIONPR(MoverFunctionExpressionManager::IsRegistered, (const string&), bool), asCALL_CDECL);

	engine->RegisterGlobalFunction(SU_IF_MSCURSOR "@ GetMusicCursor()", asMETHOD(ExecutionManager, GetMusicsManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction(SU_IF_CHARACTER_MANAGER "@ GetCharacterManager()", asMETHOD(ExecutionManager, GetCharacterManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction(SU_IF_SKILL_MANAGER "@ GetSkillManager()", asMETHOD(ExecutionManager, GetSkillManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool Execute(const string &in)", asMETHODPR(ExecutionManager, ExecuteSkin, (const string&), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool ExecuteScene(" SU_IF_SCENE "@)", asMETHODPR(ExecutionManager, ExecuteScene, (asIScriptObject*), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool ExecuteScene(" SU_IF_COSCENE "@)", asMETHODPR(ExecutionManager, ExecuteScene, (asIScriptObject*), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction(SU_IF_SOUNDMIXER "@ GetDefaultMixer(const string &in)", asMETHOD(ExecutionManager, GetDefaultMixer), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterObjectBehaviour(SU_IF_SCENE_PLAYER, asBEHAVE_FACTORY, SU_IF_SCENE_PLAYER "@ f()", asMETHOD(ExecutionManager, CreatePlayer), asCALL_THISCALL_ASGLOBAL, this);
}


void ExecutionManager::EnumerateSkins()
{
	using namespace std;
	using namespace filesystem;
	auto log = spdlog::get("main");

	const auto sepath = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR;

	for (const auto& fdata : directory_iterator(sepath)) {
		if (!is_directory(fdata)) continue;
		if (!CheckSkinStructure(fdata.path())) continue;
		skinNames.push_back(fdata.path().filename().wstring());
	}
	log->info(u8"スキン総数: {0:d}", skinNames.size());
}

bool ExecutionManager::CheckSkinStructure(const path & name) const
{
	using namespace std;
	using namespace filesystem;

	if (!exists(name / SU_SKIN_MAIN_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_TITLE_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_SELECT_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_PLAY_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_RESULT_FILE)) return false;
	return true;
}

void ExecutionManager::ExecuteSkin()
{
	using namespace std::filesystem;
	auto log = spdlog::get("main");

	const auto sn = sharedSetting->ReadValue<string>(SU_SETTING_GENERAL, SU_SETTING_SKIN, "Default");
	if (find(skinNames.begin(), skinNames.end(), ConvertUTF8ToUnicode(sn)) == skinNames.end()) {
		log->error(u8"スキン \"{0}\"が見つかりませんでした", sn);
		return;
	}
	const auto skincfg = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / ConvertUTF8ToUnicode(sn) / SU_SETTING_DEFINITION_FILE;
	if (exists(skincfg)) {
		log->info(u8"スキンの設定定義ファイルが有効です");
		settingManager->LoadItemsFromToml(skincfg);
		settingManager->RetrieveAllValues();
	}

	skin = make_unique<SkinHolder>(ConvertUTF8ToUnicode(sn), scriptInterface, sound);
	skin->Initialize();
	log->info(u8"スキン読み込み完了");
	ExecuteSkin(ConvertUnicodeToUTF8(SU_SKIN_TITLE_FILE));
}

bool ExecutionManager::ExecuteSkin(const string & file)
{
	auto log = spdlog::get("main");
	const auto obj = skin->ExecuteSkinScript(ConvertUTF8ToUnicode(file));
	if (!obj) {
		log->error(u8"スクリプトをコンパイルできませんでした");
		return false;
	}
	const auto s = CreateSceneFromScriptObject(obj);
	if (!s) {
		log->error(u8"{0}にEntryPointが見つかりませんでした", file);
		obj->Release();
		return false;
	}
	AddScene(s);

	obj->Release();
	return true;
}

bool ExecutionManager::ExecuteScene(asIScriptObject * sceneObject)
{
	auto log = spdlog::get("main");
	const auto s = CreateSceneFromScriptObject(sceneObject);
	if (!s) return false;
	sceneObject->SetUserData(skin.get(), SU_UDTYPE_SKIN);
	AddScene(s);

	sceneObject->Release();
	return true;
}

void ExecutionManager::ExecuteSystemMenu()
{
	using namespace std;
	using namespace filesystem;
	auto log = spdlog::get("main");

	auto sysmf = Setting::GetRootDirectory() / SU_DATA_DIR / SU_SCRIPT_DIR / SU_SYSTEM_MENU_FILE;
	if (!exists(sysmf)) {
		log->error(u8"システムメニュースクリプトが見つかりませんでした");
		return;
	}

	scriptInterface->StartBuildModule("SystemMenu", [](auto inc, auto from, auto sb) { return true; });
	scriptInterface->LoadFile(sysmf.wstring());
	if (!scriptInterface->FinishBuildModule()) {
		log->error(u8"システムメニュースクリプトをコンパイルできませんでした");
		return;
	}
	const auto mod = scriptInterface->GetLastModule();

	//エントリポイント検索
	const int cnt = mod->GetObjectTypeCount();
	asITypeInfo* type = nullptr;
	for (auto i = 0; i < cnt; i++) {
		const auto cti = mod->GetObjectTypeByIndex(i);
		if (!scriptInterface->CheckMetaData(cti, "EntryPoint")) continue;
		type = cti;
		type->AddRef();
		break;
	}
	if (!type) {
		log->error(u8"システムメニュースクリプトにEntryPointが見つかりませんでした");
		return;
	}

	AddScene(CreateSceneFromScriptType(type));

	type->Release();
}


//Tick
void ExecutionManager::Tick(const double delta)
{
	sharedControlState->Update();

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
		}
		else {
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

//Draw
void ExecutionManager::Draw()
{
	ClearDrawScreen();
	for (const auto& s : scenes) s->Draw();
	ScreenFlip();
}

void ExecutionManager::AddScene(const shared_ptr<Scene> & scene)
{
	scenesPending.push_back(scene);
	scene->SetManager(this);
	scene->Initialize();
}

shared_ptr<ScriptScene> ExecutionManager::CreateSceneFromScriptType(asITypeInfo * type) const
{
	auto log = spdlog::get("main");
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
	log->error(u8"{0}クラスにScene系インターフェースが実装されていません", type->GetName());
	return nullptr;
}

shared_ptr<ScriptScene> ExecutionManager::CreateSceneFromScriptObject(asIScriptObject * obj) const
{
	auto log = spdlog::get("main");
	shared_ptr<ScriptScene> ret;
	const auto type = obj->GetObjectType();
	if (scriptInterface->CheckImplementation(type, SU_IF_COSCENE)) {
		return static_pointer_cast<ScriptScene>(make_shared<ScriptCoroutineScene>(obj));
	}
	if (scriptInterface->CheckImplementation(type, SU_IF_SCENE))  //最後
	{
		return make_shared<ScriptScene>(obj);
	}
	log->error(u8"{0}クラスにScene系インターフェースが実装されていません", type->GetName());
	return nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
std::tuple<bool, LRESULT> ExecutionManager::CustomWindowProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam) const
{
	ostringstream buffer;
	switch (msg) {
	case WM_SEAURCHIN_ABORT:
		InterfacesExitApplication();
		return make_tuple(true, 0);
		/*
			//IME
		case WM_INPUTLANGCHANGE:
			WriteDebugConsole("Input Language Changed\n");
			buffer << "CharSet:" << wParam << ", Locale:" << LOWORD(lParam);
			WriteDebugConsole(buffer.str().c_str());
			return make_tuple(true, TRUE);
		case WM_IME_SETCONTEXT:
			WriteDebugConsole("Input Set Context\n");
			return make_tuple(false, 0);
		case WM_IME_STARTCOMPOSITION:
			WriteDebugConsole("Input Start Composition\n");
			return make_tuple(false, 0);
		case WM_IME_COMPOSITION:
			WriteDebugConsole("Input Conposition\n");
			return make_tuple(false, 0);
		case WM_IME_ENDCOMPOSITION:
			WriteDebugConsole("Input End Composition\n");
			return make_tuple(false, 0);
		case WM_IME_NOTIFY:
			WriteDebugConsole("Input Notify\n");
			return make_tuple(false, 0);
			*/
	default:
		return make_tuple(false, LRESULT(0));
	}

}
