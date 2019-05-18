#include "ExecutionManager.h"

#include "Setting.h"
#include "Settingmanager.h"

#include "AngelScriptManager.h"
#include "Scene.h"
#include "SceneDebug.h"
#include "ScenePlayer.h"
#include "ScriptResource.h"
#include "ScriptScene.h"
#include "ScriptSprite.h"
#include "SkinHolder.h"
#include "MusicsManager.h"
#include "Music.h"
#include "ExtensionManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "ScenePlayer.h"
#include "Controller.h"
#include "Character.h"
#include "Skill.h"
#include "MoverFunctionExpression.h"

using namespace std::filesystem;
using namespace std;

namespace {
	bool CheckSkinStructure(const path& root)
	{
		if (!exists(root / SU_SKIN_MAIN_FILE)) return false;
		if (!exists(root / SU_SCRIPT_DIR / SU_SKIN_TITLE_FILE)) return false;
		if (!exists(root / SU_SCRIPT_DIR / SU_SKIN_SELECT_FILE)) return false;
		if (!exists(root / SU_SCRIPT_DIR / SU_SKIN_PLAY_FILE)) return false;
		if (!exists(root / SU_SCRIPT_DIR / SU_SKIN_RESULT_FILE)) return false;
		return true;
	}

	ScriptScene* CreateSceneFromScriptObject(AngelScript* scriptInterface, SkinHolder* skin, asIScriptObject* obj)
	{
		if (!obj) return nullptr;

		const auto type = obj->GetObjectType();
		ScriptScene* ret = nullptr;
		if (scriptInterface->CheckImplementation(type, SU_IF_COSCENE)) {
			ret = new ScriptCoroutineScene(obj);
		}
		else if (scriptInterface->CheckImplementation(type, SU_IF_SCENE)) {
			ret = new ScriptScene(obj);
		}
		else {
			spdlog::get("main")->error(u8"{0}クラスにScene系インターフェースが実装されていません", type->GetName());
		}

		if (ret) {
			obj->SetUserData(skin, SU_UDTYPE_SKIN);
		}

		obj->Release();
		return ret;
	}
}

const static toml::Array defaultSliderKeys = {
	toml::Array { KEY_INPUT_A }, toml::Array { KEY_INPUT_Z }, toml::Array { KEY_INPUT_S }, toml::Array { KEY_INPUT_X },
	toml::Array { KEY_INPUT_D }, toml::Array { KEY_INPUT_C }, toml::Array { KEY_INPUT_F }, toml::Array { KEY_INPUT_V },
	toml::Array { KEY_INPUT_G }, toml::Array { KEY_INPUT_B }, toml::Array { KEY_INPUT_H }, toml::Array { KEY_INPUT_N },
	toml::Array { KEY_INPUT_J }, toml::Array { KEY_INPUT_M }, toml::Array { KEY_INPUT_K }, toml::Array { KEY_INPUT_COMMA }
};

const static toml::Array defaultAirStringKeys = {
	toml::Array { KEY_INPUT_PGUP }, toml::Array { KEY_INPUT_PGDN }, toml::Array { KEY_INPUT_HOME }, toml::Array { KEY_INPUT_END }
};

ExecutionManager::ExecutionManager(SettingTree* setting)
	: settingManager(new SettingManager(setting))
	, scriptInterface(new AngelScript())
	, musics(new MusicsManager())
	, characters(new CharacterManager())
	, skills(new SkillManager())
	, extensions(new ExtensionManager())
	, random(new mt19937(random_device()()))
	, sharedControlState(new ControlState)
	, hImc(nullptr)
	, hCommunicationPipe(nullptr)
	, immConversion(0)
	, immSentence(0)
{}

ExecutionManager::~ExecutionManager()
{
	for (auto& scene : scenes) scene->Disappear();
	scenes.clear();
	for (auto& scene : scenesPending) scene->Disappear();
	scenesPending.clear();

	if (skin) skin->Terminate();

	settingManager->SaveAllValues();
	sharedControlState->Terminate();

	if (hCommunicationPipe != INVALID_HANDLE_VALUE) {
		DisconnectNamedPipe(hCommunicationPipe);
		CloseHandle(hCommunicationPipe);
	}
}

void ExecutionManager::Initialize()
{
	auto log = spdlog::get("main");

	// ルートのSettingList読み込み
	const auto slpath = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SCRIPT_DIR / SU_SETTING_DEFINITION_FILE;
	settingManager->LoadItemsFromToml(slpath);
	settingManager->RetrieveAllValues();

	// 入力設定
	sharedControlState->Initialize();

	const auto setting = settingManager->GetSettingInstanceUnsafe();
	auto loadedSliderKeys = setting->ReadValue<toml::Array>("Play", "SliderKeys", defaultSliderKeys);
	if (loadedSliderKeys.size() >= 16) {
		for (auto i = 0; i < 16; i++) sharedControlState->SetSliderKeyCombination(i, loadedSliderKeys[i].as<vector<int>>());
	}
	else {
		log->warn(u8"スライダーキー設定の配列が16要素未満のため、フォールバックを利用します");
	}

	auto loadedAirStringKeys = setting->ReadValue<toml::Array>("Play", "AirStringKeys", defaultAirStringKeys);
	if (loadedAirStringKeys.size() >= 4) {
		for (auto i = 0; i < 4; i++) sharedControlState->SetAirStringKeyCombination(i, loadedAirStringKeys[i].as<vector<int>>());
	}
	else {
		log->warn(u8"エアストリングキー設定の配列が4要素未満のため、フォールバックを利用します");
	}

	// AngelScriptインターフェース登録
	const auto engine = scriptInterface->GetEngine();
	RegisterScriptResource(this);
	RegisterScriptSprite(this);
	RegisterScriptScene(this);
	SkinHolder::RegisterType(engine);
	RegisterResultTypes(engine);
	SkillManager::RegisterType(engine);
	CharacterManager::RegisterType(engine);
	engine->RegisterFuncdef("void " SU_IF_SKILL_CALLBACK "(const string &in)");
	engine->RegisterFuncdef("void " SU_IF_JUDGE_CALLBACK "(" SU_IF_JUDGE_DATA ", const string &in)");
	RegisterPlayerScene(this);
	RegisterGlobalManagementFunction();
	extensions->RegisterInterfaces();

	// 拡張ライブラリ読み込み
	extensions->LoadExtensions();
	extensions->Initialize(scriptInterface->GetEngine());

	// キャラ・スキル読み込み
	characters->LoadAllCharacters();
	skills->LoadAllSkills(engine);

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

	{
		Scene* s = new SceneDebug();
		auto ptr = unique_ptr<Scene>(s);
		scenesPending.push_back(move(ptr));
		s->SetManager(this);
		s->Initialize();
	}
}

void ExecutionManager::RegisterGlobalManagementFunction()
{
	auto engine = scriptInterface->GetEngine();

	engine->RegisterEnum(SU_IF_SEVERITY);
	engine->RegisterEnumValue(SU_IF_SEVERITY, "Trace", int(ScriptLogSeverity::Trace));
	engine->RegisterEnumValue(SU_IF_SEVERITY, "Debug", int(ScriptLogSeverity::Debug));
	engine->RegisterEnumValue(SU_IF_SEVERITY, "Info", int(ScriptLogSeverity::Info));
	engine->RegisterEnumValue(SU_IF_SEVERITY, "Warn", int(ScriptLogSeverity::Warning));
	engine->RegisterEnumValue(SU_IF_SEVERITY, "Error", int(ScriptLogSeverity::Error));
	engine->RegisterEnumValue(SU_IF_SEVERITY, "Critical", int(ScriptLogSeverity::Critical));

	engine->RegisterEnum(SU_IF_KEY);
#define RegisterKeyEnum(Key) (engine->RegisterEnumValue(SU_IF_KEY, # Key, KEY_ ## Key))
	RegisterKeyEnum(INPUT_BACK);
	RegisterKeyEnum(INPUT_TAB);
	RegisterKeyEnum(INPUT_RETURN);
	RegisterKeyEnum(INPUT_LSHIFT);
	RegisterKeyEnum(INPUT_RSHIFT);
	RegisterKeyEnum(INPUT_LCONTROL);
	RegisterKeyEnum(INPUT_RCONTROL);
	RegisterKeyEnum(INPUT_ESCAPE);
	RegisterKeyEnum(INPUT_SPACE);
	RegisterKeyEnum(INPUT_PGUP);
	RegisterKeyEnum(INPUT_PGDN);
	RegisterKeyEnum(INPUT_END);
	RegisterKeyEnum(INPUT_HOME);
	RegisterKeyEnum(INPUT_LEFT);
	RegisterKeyEnum(INPUT_UP);
	RegisterKeyEnum(INPUT_RIGHT);
	RegisterKeyEnum(INPUT_DOWN);
	RegisterKeyEnum(INPUT_INSERT);
	RegisterKeyEnum(INPUT_DELETE);
	RegisterKeyEnum(INPUT_MINUS);
	RegisterKeyEnum(INPUT_YEN);
	RegisterKeyEnum(INPUT_PREVTRACK);
	RegisterKeyEnum(INPUT_PERIOD);
	RegisterKeyEnum(INPUT_SLASH);
	RegisterKeyEnum(INPUT_LALT);
	RegisterKeyEnum(INPUT_RALT);
	RegisterKeyEnum(INPUT_SCROLL);
	RegisterKeyEnum(INPUT_SEMICOLON);
	RegisterKeyEnum(INPUT_COLON);
	RegisterKeyEnum(INPUT_LBRACKET);
	RegisterKeyEnum(INPUT_RBRACKET);
	RegisterKeyEnum(INPUT_AT);
	RegisterKeyEnum(INPUT_BACKSLASH);
	RegisterKeyEnum(INPUT_COMMA);
	RegisterKeyEnum(INPUT_KANJI);
	RegisterKeyEnum(INPUT_CONVERT);
	RegisterKeyEnum(INPUT_NOCONVERT);
	RegisterKeyEnum(INPUT_KANA);
	RegisterKeyEnum(INPUT_APPS);
	RegisterKeyEnum(INPUT_CAPSLOCK);
	RegisterKeyEnum(INPUT_SYSRQ);
	RegisterKeyEnum(INPUT_PAUSE);
	RegisterKeyEnum(INPUT_LWIN);
	RegisterKeyEnum(INPUT_RWIN);
	RegisterKeyEnum(INPUT_NUMLOCK);
	RegisterKeyEnum(INPUT_NUMPAD0);
	RegisterKeyEnum(INPUT_NUMPAD1);
	RegisterKeyEnum(INPUT_NUMPAD2);
	RegisterKeyEnum(INPUT_NUMPAD3);
	RegisterKeyEnum(INPUT_NUMPAD4);
	RegisterKeyEnum(INPUT_NUMPAD5);
	RegisterKeyEnum(INPUT_NUMPAD6);
	RegisterKeyEnum(INPUT_NUMPAD7);
	RegisterKeyEnum(INPUT_NUMPAD8);
	RegisterKeyEnum(INPUT_NUMPAD9);
	RegisterKeyEnum(INPUT_MULTIPLY);
	RegisterKeyEnum(INPUT_ADD);
	RegisterKeyEnum(INPUT_SUBTRACT);
	RegisterKeyEnum(INPUT_DECIMAL);
	RegisterKeyEnum(INPUT_DIVIDE);
	RegisterKeyEnum(INPUT_NUMPADENTER);
	RegisterKeyEnum(INPUT_F1);
	RegisterKeyEnum(INPUT_F2);
	RegisterKeyEnum(INPUT_F3);
	RegisterKeyEnum(INPUT_F4);
	RegisterKeyEnum(INPUT_F5);
	RegisterKeyEnum(INPUT_F6);
	RegisterKeyEnum(INPUT_F7);
	RegisterKeyEnum(INPUT_F8);
	RegisterKeyEnum(INPUT_F9);
	RegisterKeyEnum(INPUT_F10);
	RegisterKeyEnum(INPUT_F11);
	RegisterKeyEnum(INPUT_F12);
	RegisterKeyEnum(INPUT_A);
	RegisterKeyEnum(INPUT_B);
	RegisterKeyEnum(INPUT_C);
	RegisterKeyEnum(INPUT_D);
	RegisterKeyEnum(INPUT_E);
	RegisterKeyEnum(INPUT_F);
	RegisterKeyEnum(INPUT_G);
	RegisterKeyEnum(INPUT_H);
	RegisterKeyEnum(INPUT_I);
	RegisterKeyEnum(INPUT_J);
	RegisterKeyEnum(INPUT_K);
	RegisterKeyEnum(INPUT_L);
	RegisterKeyEnum(INPUT_M);
	RegisterKeyEnum(INPUT_N);
	RegisterKeyEnum(INPUT_O);
	RegisterKeyEnum(INPUT_P);
	RegisterKeyEnum(INPUT_Q);
	RegisterKeyEnum(INPUT_R);
	RegisterKeyEnum(INPUT_S);
	RegisterKeyEnum(INPUT_T);
	RegisterKeyEnum(INPUT_U);
	RegisterKeyEnum(INPUT_V);
	RegisterKeyEnum(INPUT_W);
	RegisterKeyEnum(INPUT_X);
	RegisterKeyEnum(INPUT_Y);
	RegisterKeyEnum(INPUT_Z);
	RegisterKeyEnum(INPUT_0);
	RegisterKeyEnum(INPUT_1);
	RegisterKeyEnum(INPUT_2);
	RegisterKeyEnum(INPUT_3);
	RegisterKeyEnum(INPUT_4);
	RegisterKeyEnum(INPUT_5);
	RegisterKeyEnum(INPUT_6);
	RegisterKeyEnum(INPUT_7);
	RegisterKeyEnum(INPUT_8);
	RegisterKeyEnum(INPUT_9);
#undef RegisterKeyEnum

	engine->RegisterGlobalFunction("void ExitApplication()", asMETHOD(ExecutionManager, ExitApplication), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void WriteLog(const string &in)", asMETHODPR(ExecutionManager, WriteLog, (const string&), void), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("void WriteLog(" SU_IF_SEVERITY ", const string &in)", asMETHODPR(ExecutionManager, WriteLog, (ScriptLogSeverity, const string&), void), asCALL_THISCALL_ASGLOBAL, this);
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

	if (!ScoreDifficulty::RegisterTypes(engine)) spdlog::get("main")->error(u8"スクリプトレジストエラー : ScoreDifficulty");
	if (!ScoreParameter::RegisterTypes(engine)) spdlog::get("main")->error(u8"スクリプトレジストエラー : ScoreParameter");
	if (!MusicParameter::RegisterTypes(engine)) spdlog::get("main")->error(u8"スクリプトレジストエラー : MusicParameter");
	if (!CategoryParameter::RegisterTypes(engine)) spdlog::get("main")->error(u8"スクリプトレジストエラー : CategoryParameter");
	MusicsManager::RegisterType(engine);

	engine->RegisterGlobalFunction(SU_IF_MUSIC_MANAGER "@ GetMusicManager()", asMETHOD(ExecutionManager, GetMusicsManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction(SU_IF_CHARACTER_MANAGER "@ GetCharacterManager()", asMETHOD(ExecutionManager, GetCharacterManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction(SU_IF_SKILL_MANAGER "@ GetSkillManager()", asMETHOD(ExecutionManager, GetSkillManagerUnsafe), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool Execute(const string &in)", asMETHODPR(ExecutionManager, ExecuteSkin, (const string&), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool ExecuteScene(" SU_IF_SCENE "@)", asMETHODPR(ExecutionManager, ExecuteScene, (asIScriptObject*), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterGlobalFunction("bool ExecuteScene(" SU_IF_COSCENE "@)", asMETHODPR(ExecutionManager, ExecuteScene, (asIScriptObject*), bool), asCALL_THISCALL_ASGLOBAL, this);
	engine->RegisterObjectBehaviour(SU_IF_SCENE_PLAYER, asBEHAVE_FACTORY, SU_IF_SCENE_PLAYER "@ f()", asMETHOD(ExecutionManager, CreatePlayer), asCALL_THISCALL_ASGLOBAL, this);

	engine->RegisterGlobalFunction("int GetIndex()", asFUNCTION(ScriptSceneGetIndex), asCALL_CDECL);
	engine->RegisterGlobalFunction("void SetIndex(int)", asFUNCTION(ScriptSceneSetIndex), asCALL_CDECL);
	engine->RegisterGlobalFunction("bool IsKeyHeld(" SU_IF_KEY ")", asFUNCTION(ScriptSceneIsKeyHeld), asCALL_CDECL);
	engine->RegisterGlobalFunction("bool IsKeyTriggered(" SU_IF_KEY ")", asFUNCTION(ScriptSceneIsKeyTriggered), asCALL_CDECL);
	engine->RegisterGlobalFunction("void RunCoroutine(" SU_IF_COROUTINE "@, const string &in)", asFUNCTION(ScriptSceneRunCoroutine), asCALL_CDECL);
	engine->RegisterGlobalFunction("void KillCoroutine(const string &in)", asFUNCTION(ScriptSceneKillCoroutine), asCALL_CDECL);
	engine->RegisterGlobalFunction("void Disappear()", asFUNCTION(ScriptSceneDisappear), asCALL_CDECL);
	engine->RegisterGlobalFunction("void AddSprite(" SU_IF_SPRITE "@)", asFUNCTION(ScriptSceneAddSprite), asCALL_CDECL);
	engine->RegisterGlobalFunction("void YieldTime(double)", asFUNCTION(YieldTime), asCALL_CDECL);
	engine->RegisterGlobalFunction("void YieldFrame(int64)", asFUNCTION(YieldFrames), asCALL_CDECL);
}


void ExecutionManager::EnumerateSkins()
{
	auto log = spdlog::get("main");

	const auto sepath = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR;

	skinNames.clear();
	if (exists(sepath)) {
		for (const auto& fdata : directory_iterator(sepath)) {
			if (!is_directory(fdata)) continue;
			if (!CheckSkinStructure(fdata.path())) continue;
			skinNames.push_back(fdata.path().filename());
		}
	}
	log->info(u8"スキン総数: {0:d}", skinNames.size());
}

bool ExecutionManager::ExecuteSkin()
{
	auto log = spdlog::get("main");

	const auto skinName = settingManager->GetSettingInstanceUnsafe()->ReadValue<string>(SU_SETTING_GENERAL, SU_SETTING_SKIN, "Default");
	const auto sn = ConvertUTF8ToUnicode(skinName);
	if (find(skinNames.begin(), skinNames.end(), sn) == skinNames.end()) {
		log->error(u8"スキン \"{0}\"が見つかりませんでした", skinName);
		return false;
	}
	const auto skincfg = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SKIN_DIR / sn / SU_SETTING_DEFINITION_FILE;
	if (exists(skincfg)) {
		log->info(u8"スキンの設定定義ファイルが有効です");
		settingManager->LoadItemsFromToml(skincfg);
		settingManager->RetrieveAllValues();
	}

	const auto ptr = SkinHolder::Create(scriptInterface, sn);
	if (!ptr) {
		log->critical(u8"スキン読み込みに失敗しました。");
		return false;
	}
	skin.reset(ptr);
	if (!ptr->Initialize()) {
		log->error(u8"スキンの開始に失敗しました。");
		skin->Terminate();
		skin.reset(nullptr);
		return false;
	}

	log->info(u8"スキン読み込み完了");
	return true;
}

bool ExecutionManager::ExecuteSkin(const string& file)
{
	if (!skin) {
		spdlog::get("main")->error(u8"スキンが起動していません");
		return false;
	}

	auto obj = skin->ExecuteSkinScript(ConvertUTF8ToUnicode(file));
	if (!obj) {
		spdlog::get("main")->error(u8"スクリプトをコンパイルできませんでした");
		return false;
	}

	obj->AddRef();
	bool result = ExecuteScene(obj);

	obj->Release();
	return result;
}

bool ExecutionManager::ExecuteScene(asIScriptObject * sceneObject)
{
	if (!sceneObject) return false;

	sceneObject->AddRef();
	const auto s = CreateSceneFromScriptObject(scriptInterface.get(), skin.get(), sceneObject);
	if (s) {
		unique_ptr<Scene> ptr(s);
		scenesPending.push_back(move(ptr));
		s->SetManager(this);
		s->Initialize();
	}

	sceneObject->Release();
	return !!s;
}


//Tick
void ExecutionManager::Tick(const double delta)
{
	sharedControlState->Update();

	//シーン操作
	for (auto& scene : scenesPending) scenes.push_back(move(scene));
	scenesPending.clear();
	sort(scenes.begin(), scenes.end(), [](const auto& sa, const auto& sb) { return sa->GetIndex() < sb->GetIndex(); });
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
	scriptInterface->GetEngine()->GarbageCollect(asGC_ONE_STEP);
}

//Draw
void ExecutionManager::Draw()
{
	ClearDrawScreen();
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
	for (const auto& s : scenes) s->Draw();
	ScreenFlip();
}


bool ExecutionManager::CustomWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) const
{
//	ostringstream buffer;
	switch (msg) {
	case WM_SEAURCHIN_ABORT:
		ExitApplication();
		*pResult = LRESULT(0);
		return true;
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
		*pResult = LRESULT(0);
		return false;
	}
}
