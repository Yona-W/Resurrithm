#include "Skill.h"
#include "Setting.h"
#include "ExecutionManager.h"

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
	if (!func || func->GetFuncType() != asFUNC_DELEGATE) return;

	asIScriptContext * ctx = asGetActiveContext();
	if (!ctx) return;

	void* p = ctx->GetUserData(SU_UDTYPE_SCENE);
	ScriptScene * sceneObj = static_cast<ScriptScene*>(p);

	if (!sceneObj) {
		ScriptSceneWarnOutOf("SkillIndicators::SetCallback", "Scene Class", ctx);
		return;
	}

	if (callback) callback->Release();

	func->AddRef();
	callback = new CallbackObject(func);
	callback->SetUserData(sceneObj, SU_UDTYPE_SCENE);

	callback->AddRef();
	sceneObj->RegisterDisposalCallback(callback);

	func->Release();
}

int SkillIndicators::AddSkillIndicator(const string & icon)
{
	using namespace std::filesystem;
	auto path = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR / ConvertUTF8ToUnicode(icon);
	const auto image = SImage::CreateLoadedImageFromFile(ConvertUnicodeToUTF8(path.wstring()), true);
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
	callback->SetArg(0, index);
	callback->Execute();
	callback->Unprepare();
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
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnJusticeCritical(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnJustice(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnAttack(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");
	engine->RegisterInterfaceMethod(SU_IF_ABILITY, "void OnMiss(" SU_IF_RESULT "@, " SU_IF_JUDGE_DATA ")");

	engine->RegisterObjectType(SU_IF_SKILL, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty(SU_IF_SKILL, "string Name", asOFFSET(SkillParameter, Name));
	engine->RegisterObjectProperty(SU_IF_SKILL, "string IconPath", asOFFSET(SkillParameter, IconPath));
	engine->RegisterObjectProperty(SU_IF_SKILL, "int CurrentLevel", asOFFSET(SkillParameter, CurrentLevel));
	engine->RegisterObjectMethod(SU_IF_SKILL, "string GetDescription(int)", asMETHOD(SkillParameter, GetDescription), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL, "int GetMaxLevel()", asMETHOD(SkillParameter, GetMaxLevel), asCALL_THISCALL);
}
