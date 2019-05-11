#include "Skill.h"
#include "Result.h"
#include "SettingManager.h"
#include "ExecutionManager.h"
#include "ScriptSprite.h"
#include "ScriptScene.h"
#include "CharacterInstance.h"
#include "AngelScriptManager.h"

using namespace std;

SkillIndicators::SkillIndicators()
	: callback(nullptr)
{}

SkillIndicators::~SkillIndicators()
{
	for (const auto& i : indicatorIcons) i->Release();
	if (callback) callback->Release();
}

void SkillIndicators::SetCallback(asIScriptFunction * func)
{
	asIScriptContext * ctx = asGetActiveContext();
	if (!ctx) {
		if (func) func->Release();
		return;
	}

	void* p = ctx->GetUserData(SU_UDTYPE_SCENE);
	ScriptScene * sceneObj = static_cast<ScriptScene*>(p);

	if (!sceneObj) {
		ScriptSceneWarnOutOf("SkillIndicators::SetCallback", "Scene Class", ctx);
		if (func) func->Release();
		return;
	}

	if (callback) callback->Release();

	if (!func) return;

	func->AddRef();
	callback = CallbackObject::Create(func);

	func->Release();
	if (!callback) return;

	callback->SetUserData(sceneObj, SU_UDTYPE_SCENE);

	callback->AddRef();
	sceneObj->RegisterDisposalCallback(callback);
}

int SkillIndicators::AddSkillIndicator(const string & icon)
{
	using namespace std::filesystem;
	auto path = SettingManager::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR / ConvertUTF8ToUnicode(icon);
	const auto image = SImage::CreateLoadedImageFromFile(path, true);
	indicatorIcons.push_back(image);
	return indicatorIcons.size() - 1;
}

void SkillIndicators::TriggerSkillIndicator(const int index) const
{
	if (!callback) return;
	if (!callback->IsExists()) {
		callback->Release();
		callback = nullptr;
		return;
	}

	callback->Prepare();
	if (callback->SetArgument([&index](auto p) {
		p->SetArgDWord(0, index);
		return true;
		})) {
		if (callback->Execute() != asEXECUTION_FINISHED) {
			callback->Dispose();
			callback->Release();
			callback = nullptr;
		}
		callback->Unprepare();
	}
}

uint32_t SkillIndicators::GetSkillIndicatorCount() const
{
	return indicatorIcons.size();
}

SImage* SkillIndicators::GetSkillIndicatorImage(const uint32_t index)
{
	if (index >= indicatorIcons.size()) return nullptr;
	auto result = indicatorIcons[index];

	result->AddRef();
	return result;
}


Ability::Ability(MethodObject* methodInitialize, MethodObject* methodOnStart, MethodObject* methodOnFinish, MethodObject* methodOnJudge)
	: methodInitialize(methodInitialize)
	, methodOnStart(methodOnStart)
	, methodOnFinish(methodOnFinish)
	, methodOnJudge(methodOnJudge)
{}

Ability::~Ability()
{
	if (methodInitialize) delete methodInitialize;
	if (methodOnStart) delete methodOnStart;
	if (methodOnFinish) delete methodOnFinish;
	if (methodOnJudge) delete methodOnJudge;
}

Ability* Ability::Create(asIScriptObject* obj)
{
	if (!obj) return nullptr;

	obj->AddRef();
	const auto initialize = MethodObject::Create(obj, "void Initialize(dictionary@, " SU_IF_SKILL_INDICATORS "@)");
	obj->AddRef();
	const auto onStart = MethodObject::Create(obj, "void OnStart(" SU_IF_RESULT "@)");
	obj->AddRef();
	const auto onFinish = MethodObject::Create(obj, "void OnFinish(" SU_IF_RESULT "@)");
	obj->AddRef();
	const auto onJudge = MethodObject::Create(obj, "void OnJudge(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");

	if (!initialize && !onStart && !onFinish && !onJudge) {
		obj->Release();
		return nullptr;
	}

	const auto ptr = new Ability(initialize, onStart, onFinish, onJudge);

	obj->Release();
	return ptr;
}

bool Ability::Initialize(CScriptDictionary* args, SkillIndicators* indicators)
{
	if (!methodInitialize) return true;

	methodInitialize->Prepare();
	if (methodInitialize->SetArgument([args, indicators](auto p) {
		p->SetArgAddress(0, args);
		p->SetArgAddress(1, indicators);
		return true;
		})) {
		if (methodInitialize->Execute() != asEXECUTION_FINISHED) {
			delete methodInitialize;
			methodInitialize = nullptr;
		}
		methodInitialize->Unprepare();
		return true;
	}

	return false;
}

bool Ability::OnStart(Result* result)
{
	if (!methodOnStart) return true;

	methodOnStart->Prepare();
	if (methodOnStart->SetArgument([result](auto p) { p->SetArgAddress(0, result); return true; })) {
		if (methodOnStart->Execute() != asEXECUTION_FINISHED) {
			delete methodOnStart;
			methodOnStart = nullptr;
		}
		methodOnStart->Unprepare();
		return true;
	}

	return false;
}

bool Ability::OnFinish(Result* result)
{
	if (!methodOnFinish) return true;

	methodOnFinish->Prepare();
	if (methodOnFinish->SetArgument([result](auto p) { p->SetArgAddress(0, result); return true; })) {
		if (methodOnFinish->Execute() != asEXECUTION_FINISHED) {
			delete methodOnFinish;
			methodOnFinish = nullptr;
		}
		methodOnFinish->Unprepare();
		return true;
	}

	return false;
}

bool Ability::OnJudge(Result* result, JudgeInformation* judgeInfo)
{
	if (!methodOnJudge) return true;

	methodOnJudge->Prepare();
	if (methodOnJudge->SetArgument([result, judgeInfo](auto p) {
		p->SetArgAddress(0, result);
		p->SetArgObject(1, judgeInfo);
		return true;
		})) {
		if (methodOnJudge->Execute() != asEXECUTION_FINISHED) {
			delete methodOnJudge;
			methodOnJudge = nullptr;
		}
		methodOnJudge->Unprepare();
		return true;
	}

	return false;
}


void RegisterSkillTypes(asIScriptEngine * engine)
{
	engine->RegisterEnum(SU_IF_NOTETYPE);
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "Tap", int(AbilityNoteType::Tap));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "ExTap", int(AbilityNoteType::ExTap));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "AwesomeExTap", int(AbilityNoteType::AwesomeExTap));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "Flick", int(AbilityNoteType::Flick));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "HellTap", int(AbilityNoteType::HellTap));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "Air", int(AbilityNoteType::Air));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "Hold", int(AbilityNoteType::Hold));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "Slide", int(AbilityNoteType::Slide));
	engine->RegisterEnumValue(SU_IF_NOTETYPE, "AirAction", int(AbilityNoteType::AirAction));

	engine->RegisterEnum(SU_IF_JUDGETYPE);
	engine->RegisterEnumValue(SU_IF_JUDGETYPE, "JusticeCritical", int(AbilityJudgeType::JusticeCritical));
	engine->RegisterEnumValue(SU_IF_JUDGETYPE, "Justice", int(AbilityJudgeType::Justice));
	engine->RegisterEnumValue(SU_IF_JUDGETYPE, "Attack", int(AbilityJudgeType::Attack));
	engine->RegisterEnumValue(SU_IF_JUDGETYPE, "Miss", int(AbilityJudgeType::Miss));

	engine->RegisterObjectType(SU_IF_JUDGE_DATA, sizeof(JudgeInformation), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<JudgeInformation>());
	engine->RegisterObjectProperty(SU_IF_JUDGE_DATA, SU_IF_JUDGETYPE " Judge", asOFFSET(JudgeInformation, JudgeType));
	engine->RegisterObjectProperty(SU_IF_JUDGE_DATA, SU_IF_NOTETYPE " Note", asOFFSET(JudgeInformation, Note));
	engine->RegisterObjectProperty(SU_IF_JUDGE_DATA, "double Left", asOFFSET(JudgeInformation, Left));
	engine->RegisterObjectProperty(SU_IF_JUDGE_DATA, "double Right", asOFFSET(JudgeInformation, Right));

	engine->RegisterFuncdef("void " SU_IF_SKILL_CALLBACK "(int)");
	engine->RegisterObjectType(SU_IF_SKILL_INDICATORS, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_SKILL_INDICATORS, "int AddIndicator(const string &in)", asMETHOD(SkillIndicators, AddSkillIndicator), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_INDICATORS, "void TriggerIndicator(int)", asMETHOD(SkillIndicators, TriggerSkillIndicator), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_INDICATORS, "void SetCallback(" SU_IF_SKILL_CALLBACK "@)", asMETHOD(SkillIndicators, SetCallback), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_INDICATORS, "uint GetIndicatorCount()", asMETHOD(SkillIndicators, GetSkillIndicatorCount), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_INDICATORS, SU_IF_IMAGE "@ GetIndicatorImage(uint)", asMETHOD(SkillIndicators, GetSkillIndicatorImage), asCALL_THISCALL);

	engine->RegisterInterface(SU_IF_ABILITY);
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void Initialize(dictionary@, " SU_IF_SKILL_INDICATORS "@)");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnStart(" SU_IF_RESULT "@)");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnFinish(" SU_IF_RESULT "@)");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnJudge(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");

	engine->RegisterObjectType(SU_IF_SKILL, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty(SU_IF_SKILL, "string Name", asOFFSET(SkillParameter, Name));
	engine->RegisterObjectProperty(SU_IF_SKILL, "string IconPath", asOFFSET(SkillParameter, IconPath));
	engine->RegisterObjectProperty(SU_IF_SKILL, "int CurrentLevel", asOFFSET(SkillParameter, CurrentLevel));
	engine->RegisterObjectMethod(SU_IF_SKILL, "string GetDescription(int)", asMETHOD(SkillParameter, GetDescription), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL, "int GetMaxLevel()", asMETHOD(SkillParameter, GetMaxLevel), asCALL_THISCALL);
}
