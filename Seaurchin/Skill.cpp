#include "Skill.h"
#include "Result.h"
#include "SettingManager.h"
#include "ExecutionManager.h"
#include "ScriptSprite.h"
#include "ScriptScene.h"
#include "AngelScriptManager.h"

using namespace std;

namespace {
	bool AbilityTriggerIndicator(const string& key)
	{
		const auto ctx = asGetActiveContext();
		const auto pab = static_cast<Ability*>(ctx->GetUserData(SU_UDTYPE_ABILITY));
		if (!pab) {
			ScriptSceneWarnOutOf("TriggerIndicator", "Ability Class", ctx);
			return false;
		}
		return  pab->CallAbilityCallback(key);
	}
}


Ability::Ability(MethodObject* methodInitialize, MethodObject* methodOnStart, MethodObject* methodOnFinish, MethodObject* methodOnJudge)
	: methodInitialize(methodInitialize)
	, methodOnStart(methodOnStart)
	, methodOnFinish(methodOnFinish)
	, methodOnJudge(methodOnJudge)
{
	if (methodInitialize) methodInitialize->SetUserData(this, SU_UDTYPE_ABILITY);
	if (methodOnStart) methodOnStart->SetUserData(this, SU_UDTYPE_ABILITY);
	if (methodOnFinish) methodOnFinish->SetUserData(this, SU_UDTYPE_ABILITY);
	if (methodOnJudge) methodOnJudge->SetUserData(this, SU_UDTYPE_ABILITY);
}

Ability::~Ability()
{
	if (methodInitialize) delete methodInitialize;
	if (methodOnStart) delete methodOnStart;
	if (methodOnFinish) delete methodOnFinish;
	if (methodOnJudge) delete methodOnJudge;
	if (judgeCallback) judgeCallback->Release();
	if (abilityCallback) abilityCallback->Release();
}

Ability* Ability::Create(asIScriptObject* obj)
{
	if (!obj) return nullptr;

	obj->AddRef();
	const auto initialize = MethodObject::Create(obj, "void Initialize(dictionary@, dictionary@)");

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

bool Ability::Initialize(const SkillDetail* detail)
{
	SU_ASSERT(detail);
	if (!methodInitialize) return true;

	methodInitialize->Prepare();
	const auto args = detail->Arguments;
	const auto indicators = detail->Indicators;
	args->AddRef();
	indicators->AddRef();
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
			return false;
		}
		methodOnJudge->Unprepare();

		if (!CallJudgeCallback(judgeInfo)) return false;

		return true;
	}

	return false;
}

bool Ability::SetJudgeCallback(CallbackObject* callback)
{
	if (judgeCallback) {
		judgeCallback->Release();
		judgeCallback = nullptr;
	}

	if (!callback) return false;
	judgeCallback = callback;

	return true;
}

bool Ability::SetAbilityCallback(CallbackObject* callback)
{
	if (abilityCallback) {
		abilityCallback->Release();
		abilityCallback = nullptr;
	}

	if (!callback) return false;
	abilityCallback = callback;

	return true;
}

bool Ability::CallJudgeCallback(JudgeInformation* judgeInfo)
{
	if (!judgeCallback) return true;

	if (!judgeCallback->IsExists()) {
		judgeCallback->Release();
		judgeCallback = nullptr;
		return true;
	}

	judgeCallback->Prepare();
	if (judgeCallback->SetArgument([judgeInfo](auto p) {
		p->SetArgObject(0, judgeInfo);
		return true;
		})) {
		if (judgeCallback->Execute() != asEXECUTION_FINISHED) {
			judgeCallback->Dispose();
			judgeCallback->Release();
			judgeCallback = nullptr;
			return false;
		}
		judgeCallback->Unprepare();
	}

	return true;
}

bool Ability::CallAbilityCallback(const string& key)
{
	if (!abilityCallback) return true;

	if (!abilityCallback->IsExists()) {
		abilityCallback->Release();
		abilityCallback = nullptr;
		return true;
	}

	abilityCallback->Prepare();
	auto keyClone = key;
	if (abilityCallback->SetArgument([&keyClone](auto p) {
		p->SetArgAddress(0, &keyClone);
		return true;
		})) {
		if (abilityCallback->Execute() != asEXECUTION_FINISHED) {
			abilityCallback->Dispose();
			abilityCallback->Release();
			abilityCallback = nullptr;
			return false;
		}
		abilityCallback->Unprepare();
	}

	return true;
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

	engine->RegisterInterface(SU_IF_ABILITY);
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void Initialize(dictionary@, dictionary@)");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnStart(" SU_IF_RESULT "@)");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnFinish(" SU_IF_RESULT "@)");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnJudge(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");

	engine->RegisterObjectType(SU_IF_SKILL_DETAIL, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty(SU_IF_SKILL_DETAIL, "int Level", asOFFSET(SkillDetail, Level));
	engine->RegisterObjectProperty(SU_IF_SKILL_DETAIL, "string Description", asOFFSET(SkillDetail, Description));
	engine->RegisterObjectProperty(SU_IF_SKILL_DETAIL, "dictionary@ Arguments", asOFFSET(SkillDetail, Arguments));
	engine->RegisterObjectProperty(SU_IF_SKILL_DETAIL, "dictionary@ Indicators", asOFFSET(SkillDetail, Indicators));

	engine->RegisterObjectType(SU_IF_SKILL, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty(SU_IF_SKILL, "string Name", asOFFSET(SkillParameter, Name));
	engine->RegisterObjectProperty(SU_IF_SKILL, "string IconPath", asOFFSET(SkillParameter, IconPath));
	engine->RegisterObjectMethod(SU_IF_SKILL, "int GetMaxLevel()", asMETHOD(SkillParameter, GetMaxLevel), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL, SU_IF_SKILL_DETAIL "@ GetDetail(int)", asMETHOD(SkillParameter, GetDetail), asCALL_THISCALL);

	engine->RegisterGlobalFunction("bool TriggerIndicator(const string &in)", asFUNCTION(AbilityTriggerIndicator), asCALL_CDECL);
}
