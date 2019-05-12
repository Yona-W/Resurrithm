#include "ExecutionManager.h"

#include "Setting.h"
#include "Settingmanager.h"
#include "Interfaces.h"

#include "AngelScriptManager.h"
#include "Scene.h"
#include "ScenePlayer.h"
#include "ScriptResource.h"
#include "ScriptScene.h"
#include "ScriptSprite.h"
#include "SkinHolder.h"
#include "MusicsManager.h"
#include "ExtensionManager.h"
#include "SoundManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "ScenePlayer.h"
#include "Controller.h"
#include "CharacterInstance.h"
#include "Character.h"
#include "Skill.h"
#include "MoverFunctionExpression.h"

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

ExecutionManager::ExecutionManager(SettingTree* setting)
	: settingManager(new SettingManager(setting))
	, scriptInterface(new AngelScript())
	, musics(new MusicsManager())
	, characters(new CharacterManager())
	, skills(new SkillManager())
	, extensions(new ExtensionManager())
	, random(new mt19937(random_device()()))
	, sharedControlState(new ControlState)
	, lastResult(new DrawableResult())
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

	// 拡張ライブラリ読み込み
	extensions->LoadExtensions();
	extensions->Initialize(scriptInterface->GetEngine());

	// AngelScriptインターフェース登録
	const auto engine = scriptInterface->GetEngine();
	InterfacesRegisterEnum(this);
	RegisterScriptResource(this);
	RegisterScriptSprite(this);
	RegisterScriptScene(this);
	SkinHolder::RegisterType(engine);
	MusicsManager::RegisterType(engine);
	RegisterCharacterSkillTypes(engine);
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

void ExecutionManager::RegisterGlobalManagementFunction()
{
	auto engine = scriptInterface->GetEngine();

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
	engine->RegisterObjectBehaviour(SU_IF_SCENE_PLAYER, asBEHAVE_FACTORY, SU_IF_SCENE_PLAYER "@ f()", asMETHOD(ExecutionManager, CreatePlayer), asCALL_THISCALL_ASGLOBAL, this);
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

bool ExecutionManager::CheckSkinStructure(const path & name) const
{
	if (!exists(name / SU_SKIN_MAIN_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_TITLE_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_SELECT_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_PLAY_FILE)) return false;
	if (!exists(name / SU_SCRIPT_DIR / SU_SKIN_RESULT_FILE)) return false;
	return true;
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

	log->info(u8"スキン読み込み完了");
	return ExecuteSkin(ConvertUnicodeToUTF8(SU_SKIN_TITLE_FILE));
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
	const auto s = CreateSceneFromScriptObject(sceneObject);
	if (s) AddScene(s);

	sceneObject->Release();
	return !!s;
}

bool ExecutionManager::ExecuteSystemMenu()
{
	const auto root = SettingManager::GetRootDirectory() / SU_DATA_DIR / SU_SCRIPT_DIR;
	const auto obj = scriptInterface->ExecuteScriptAsObject(root, SU_SYSTEM_MENU_FILE, true);
	if (!obj) return false;

	obj->AddRef();
	const auto scene = CreateSceneFromScriptObject(obj);
	if (scene) AddScene(scene);

	obj->Release();
	return !!scene;
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

void ExecutionManager::AddScene(Scene* scene)
{
	if (!scene) return;

	unique_ptr<Scene> ptr(scene);
	scenesPending.push_back(move(ptr));
	scene->SetManager(this);
	scene->Initialize();
}

ScriptScene* ExecutionManager::CreateSceneFromScriptObject(asIScriptObject* obj) const
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
		obj->SetUserData(skin.get(), SU_UDTYPE_SKIN);
	}

	obj->Release();
	return ret;
}

bool ExecutionManager::CustomWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) const
{
//	ostringstream buffer;
	switch (msg) {
	case WM_SEAURCHIN_ABORT:
		InterfacesExitApplication();
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
