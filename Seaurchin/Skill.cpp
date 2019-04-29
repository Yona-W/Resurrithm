#include "Skill.h"
#include "Setting.h"
#include "ExecutionManager.h"

using namespace std;

SkillManager::SkillManager()
{
	selected = -1;
}

void SkillManager::LoadAllSkills()
{
	using namespace filesystem;
	const auto skillroot = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_SKILL_DIR;

	if (exists(skillroot)) {
		for (const auto& fdata : directory_iterator(skillroot)) {
			if (is_directory(fdata)) continue;
			if (fdata.path().extension() != ".toml") continue;
			LoadFromToml(fdata.path());
		}
	}

	const auto size = skills.size();
	spdlog::get("main")->info(u8"スキル総数: {0:d}", size);
	selected = (size == 0) ? -1 : 0;
}

void SkillManager::Next()
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return;

	selected = (selected + 1) % size;
}

void SkillManager::Previous()
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return;

	selected = (selected + size - 1) % size;
}

SkillParameter * SkillManager::GetSkillParameter(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	return skills[ri % size].get();
}

shared_ptr<SkillParameter> SkillManager::GetSkillParameterSafe(const int relative)
{
	const auto size = GetSize();
	if (size <= 0 || selected < 0) return nullptr;

	auto ri = selected + relative;
	while (ri < 0) ri += size;
	return skills[ri % size];
}

void SkillManager::LoadFromToml(std::filesystem::path file)
{
	using namespace filesystem;
	auto log = spdlog::get("main");
	auto result = make_shared<SkillParameter>();
	const auto iconRoot = Setting::GetRootDirectory() / SU_SKILL_DIR / SU_ICON_DIR;

	if (!exists(iconRoot)) return;

	std::ifstream ifs(file.wstring(), ios::in);
	auto pr = toml::parse(ifs);
	ifs.close();
	if (!pr.valid()) {
		log->error(u8"スキル {0} は不正なファイルです", ConvertUnicodeToUTF8(file.wstring()));
		log->error(pr.errorReason);
		return;
	}
	auto& root = pr.value;

	try {
		result->Name = root.get<string>("Name");
		result->IconPath = ConvertUnicodeToUTF8((iconRoot / ConvertUTF8ToUnicode(root.get<string>("Icon"))).wstring());
		result->Details.clear();
		// TODO: CurrentLevel を変更する手段を提供する
		result->CurrentLevel = 0;
		result->MaxLevel = 0;

		auto details = root.get<vector<toml::Table>>("Detail");
		for (const auto& detail : details) {
			SkillDetail sdt;
			sdt.Description = detail.at("Description").as<string>();

			auto abilities = detail.at("Abilities").as<vector<toml::Table>>();
			for (const auto& ability : abilities) {
				AbilityParameter ai;
				ai.Name = ability.at("Type").as<string>();
				auto args = ability.at("Arguments").as<toml::Table>();
				for (const auto& p : args) {
					switch (p.second.type()) {
					case toml::Value::INT_TYPE:
						ai.Arguments[p.first] = p.second.as<int>();
						break;
					case toml::Value::DOUBLE_TYPE:
						ai.Arguments[p.first] = p.second.as<double>();
						break;
					case toml::Value::STRING_TYPE:
						ai.Arguments[p.first] = p.second.as<string>();
						break;
					default:
						break;
					}
				}
				sdt.Abilities.push_back(ai);
			}

			auto level = detail.at("Level").as<int>();
			result->Details.emplace(level, sdt);

			if (result->MaxLevel < level) result->MaxLevel = level;
		}
	}
	catch (exception & ex) {
		log->error(u8"スキル {0} の読み込みに失敗しました: {1}", ConvertUnicodeToUTF8(file.wstring()), ex.what());
		return;
	}
	skills.push_back(result);
}


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

	engine->RegisterObjectType(SU_IF_SKILL_MANAGER, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, "void Next()", asMETHOD(SkillManager, Next), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, "void Previous()", asMETHOD(SkillManager, Previous), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, SU_IF_SKILL "@ GetSkill(int)", asMETHOD(SkillManager, GetSkillParameter), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SKILL_MANAGER, "int GetSize()", asMETHOD(SkillManager, GetSize), asCALL_THISCALL);
}
