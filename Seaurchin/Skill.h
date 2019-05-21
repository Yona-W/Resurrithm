#pragma once

#define SU_IF_ABILITY "Ability"
#define SU_IF_SKILL "Skill"
#define SU_IF_SKILL_DETAIL "SkillDetail"
#define SU_IF_SKILL_CALLBACK "SkillCallback"
#define SU_IF_NOTETYPE "NoteType"
#define SU_IF_JUDGETYPE "JudgeType"
#define SU_IF_JUDGE_DATA "JudgeData"

#define SU_IF_CHARACTER_INSTANCE "CharacterInstance"
#define SU_IF_JUDGE_CALLBACK "JudgeCallback"

#define SU_CHAR_SMALL_WIDTH 280
#define SU_CHAR_SMALL_HEIGHT 170
#define SU_CHAR_FACE_SIZE 128

class SImage;
class CallbackObject;

class SkillDetail final {
public:
	int32_t Level;
	std::string Description;
	std::string AbilityName;
	CScriptDictionary* Arguments;
	CScriptDictionary* Indicators;

	~SkillDetail() {
		if (Arguments) Arguments->Release();
		if (Indicators) Indicators->Release();
	}
};

class SkillParameter final {
public:
	int32_t GetMaxLevel() { return MaxLevel; }
	SkillDetail* GetDetail(int32_t level)
	{
		if (Details.size() == 0) return new SkillDetail();

		if (level <= 0) return Details.front();

		auto l = level;
		if (l > MaxLevel) l = MaxLevel;
		while (l >= 0) {
			const auto d = std::find_if(Details.begin(), Details.end(), [l](const auto& x) { return x->Level == l; });
			if (d != Details.end()) return *d;
			--l;
		}
		return Details.front();
	}
	std::string GetDescription(int32_t level) { return GetDetail(level)->Description; }
	CScriptDictionary* GetArguments(int32_t level) { const auto& detail = GetDetail(level); detail->Arguments->AddRef(); return detail->Arguments; }
	CScriptDictionary* GetIndicators(int32_t level) { const auto& detail = GetDetail(level); detail->Indicators->AddRef(); return detail->Indicators; }

public:
	std::string Name;
	std::string IconPath;
	std::vector<SkillDetail*> Details;
	int32_t CurrentLevel;
	int32_t MaxLevel;

public:
	~SkillParameter() {
		for (auto p : Details) delete p;
	}
};

enum class AbilityNoteType {
	Tap = 1,
	ExTap,
	AwesomeExTap,
	Flick,
	Air,
	HellTap,
	Hold,
	Slide,
	AirAction,
};

enum class AbilityJudgeType {
	JusticeCritical = 1,
	Justice,
	Attack,
	Miss,
};

struct JudgeInformation {
	AbilityJudgeType JudgeType;
	AbilityNoteType Note;
	double Left;
	double Right;
};


class MethodObject;
class CallbackObject;
class Result;
class ScriptScene;

class Ability {
private:
	MethodObject* methodInitialize;
	MethodObject* methodOnStart;
	MethodObject* methodOnFinish;
	MethodObject* methodOnJudge;
	CallbackObject* judgeCallback;
	CallbackObject* abilityCallback;

private:
	Ability(MethodObject*, MethodObject*, MethodObject*, MethodObject*);
public:
	~Ability();

public:
	static Ability* Create(asIScriptObject*);

	bool Initialize(const SkillDetail*);
	bool OnStart(Result* result);
	bool OnFinish(Result* result);
	bool OnJudge(Result* result, JudgeInformation* judgeInfo);
	bool SetJudgeCallback(CallbackObject* callback);
	bool SetAbilityCallback(CallbackObject* callback);
	bool CallJudgeCallback(JudgeInformation* judgeInfo);
	bool CallAbilityCallback(const std::string& key);
};

void RegisterSkillTypes(asIScriptEngine* engine);
