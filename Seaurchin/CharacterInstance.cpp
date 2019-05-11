#include "CharacterInstance.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "SettingManager.h"
#include "ScriptScene.h"
#include "AngelScriptManager.h"
#include "Character.h"
#include "Skill.h"
#include "Result.h"

using namespace std;


CharacterInstance::CharacterInstance(const shared_ptr<CharacterParameter>& character, const shared_ptr<SkillParameter>& skill, const shared_ptr<AngelScript>& script, const shared_ptr<Result>& result)
	: scriptInterface(script)
	, characterSource(character)
	, skillSource(skill)
	, imageSet(nullptr)
	, indicators(new SkillIndicators())
	, targetResult(result)
	, judgeCallback(nullptr)
{}

CharacterInstance::~CharacterInstance()
{
	if (judgeCallback) judgeCallback->Release();
	for (auto& ability : abilities) if (ability) delete ability;
	abilities.clear();
	if (imageSet) imageSet->Release();
}

shared_ptr<CharacterInstance> CharacterInstance::CreateInstance(shared_ptr<CharacterParameter> character, shared_ptr<SkillParameter> skill, shared_ptr<AngelScript> script, std::shared_ptr<Result> result)
{
	auto ci = make_shared<CharacterInstance>(character, skill, script, result);
	ci->LoadAbilities();
	ci->CreateImageSet();
	ci->AddRef();
	return ci;
}

void CharacterInstance::LoadAbilities()
{
	using namespace filesystem;
	auto log = spdlog::get("main");

	for (auto& ability : abilities) if (ability) delete ability;
	abilities.clear();

	if (!skillSource) {
		log->info(u8"有効なスキルが選択されていません。");
		return;
	}

	const auto abroot = SettingManager::GetRootDirectory() / SU_SKILL_DIR / SU_ABILITY_DIR;
	const auto& ablist = skillSource->GetDetail(skillSource->CurrentLevel).Abilities;
	for (const auto& def : ablist) {
		vector<string> params;
		auto scrpath = ConvertUTF8ToUnicode(def.Name + ".as");

		const auto abroot = SettingManager::GetRootDirectory() / SU_SKILL_DIR / SU_ABILITY_DIR;
		const auto abo = scriptInterface->ExecuteScriptAsObject(abroot, scrpath, true);
		if (!abo) continue;

		abo->AddRef();
		auto ptr = Ability::Create(abo);
		if (!ptr) continue;
		abilities.push_back(ptr);

		auto args = CScriptDictionary::Create(scriptInterface->GetEngine());
		for (const auto& arg : def.Arguments) {
			const auto key = arg.first;
			auto value = arg.second;
			auto& vid = value.type();
			if (vid == typeid(int)) {
				asINT64 avalue = std::any_cast<int>(value);
				args->Set(key, avalue);
			}
			else if (vid == typeid(double)) {
				auto avalue = std::any_cast<double>(value);
				args->Set(key, avalue);
			}
			else if (vid == typeid(string)) {
				auto avalue = std::any_cast<string>(value);
				args->Set(key, &avalue, scriptInterface->GetEngine()->GetTypeIdByDecl("string"));
			}
		}

		ptr->Initialize(args, indicators.get());
		log->info(u8"アビリティー " + ConvertUnicodeToUTF8(scrpath));
	}
}

void CharacterInstance::CreateImageSet()
{
	if (imageSet) imageSet->Release();
	imageSet = nullptr;

	if (!characterSource) {
		spdlog::get("main")->info(u8"有効なキャラクターが選択されていません。");
		return;
	}

	imageSet = CharacterImageSet::CreateImageSet(characterSource);
}

void CharacterInstance::CallJudgeCallback(const JudgeInformation & info, const string & extra) const
{
	if (!judgeCallback) return;
	if (!judgeCallback->IsExists()) {
		judgeCallback->Release();
		judgeCallback = nullptr;
		return;
	}

	auto message = extra;
	auto infoClone = info;

	judgeCallback->Prepare();
	if (judgeCallback->SetArgument([&infoClone, &message](auto p) {
		p->SetArgObject(0, &infoClone);
		p->SetArgObject(1, &message);
		return true;
		})) {
		if (judgeCallback->Execute() != asEXECUTION_FINISHED) {
			judgeCallback->Dispose();
			judgeCallback->Release();
			judgeCallback = nullptr;
		}
		judgeCallback->Unprepare();
	}
}

void CharacterInstance::OnStart()
{
	const auto pResult = targetResult.get();
	for (auto& ability : abilities) { ability->OnStart(pResult); }
}

void CharacterInstance::OnFinish()
{
	const auto pResult = targetResult.get();
	for (auto& ability : abilities) { ability->OnFinish(pResult); }
}

void CharacterInstance::OnJudge(const JudgeInformation & info, const string & extra)
{
	auto infoClone = info;
	const auto pResult = targetResult.get();
	for (auto& ability : abilities) { ability->OnJudge(pResult, &infoClone); }
	CallJudgeCallback(info, extra);
}

void CharacterInstance::SetCallback(asIScriptFunction * func, ScriptScene * sceneObj)
{
	if (judgeCallback) judgeCallback->Release();

	if (!func) return;

	func->AddRef();
	judgeCallback = CallbackObject::Create(func);

	func->Release();
	if (!judgeCallback) return;

	judgeCallback->SetUserData(sceneObj, SU_UDTYPE_SCENE);

	judgeCallback->AddRef();
	sceneObj->RegisterDisposalCallback(judgeCallback);
}

CharacterParameter* CharacterInstance::GetCharacterParameter() const
{
	return characterSource ? characterSource.get() : nullptr;
}

SkillParameter* CharacterInstance::GetSkillParameter() const
{
	return skillSource ? skillSource.get() : nullptr;
}

SkillIndicators* CharacterInstance::GetSkillIndicators() const
{
	return indicators.get();
}

CharacterImageSet* CharacterInstance::GetCharacterImages() const
{
	if (imageSet) imageSet->AddRef();
	return imageSet;
}

void RegisterCharacterSkillTypes(asIScriptEngine * engine)
{
	RegisterResultTypes(engine);
	SkillManager::RegisterType(engine);
	CharacterManager::RegisterType(engine);

	engine->RegisterFuncdef("void " SU_IF_JUDGE_CALLBACK "(" SU_IF_JUDGE_DATA ", const string &in)");
	engine->RegisterObjectType(SU_IF_CHARACTER_INSTANCE, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_CHARACTER_INSTANCE, asBEHAVE_ADDREF, "void f()", asMETHOD(CharacterInstance, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_CHARACTER_INSTANCE, asBEHAVE_RELEASE, "void f()", asMETHOD(CharacterInstance, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_CHARACTER_PARAM "@ GetCharacter()", asMETHOD(CharacterInstance, GetCharacterParameter), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_CHARACTER_IMAGES "@ GetCharacterImages()", asMETHOD(CharacterInstance, GetCharacterImages), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_SKILL "@ GetSkill()", asMETHOD(CharacterInstance, GetSkillParameter), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_INSTANCE, SU_IF_SKILL_INDICATORS "@ GetSkillIndicators()", asMETHOD(CharacterInstance, GetSkillIndicators), asCALL_THISCALL);
}
