#pragma once

#define SU_IF_ABILITY "Ability"
#define SU_IF_SKILL "Skill"
#define SU_IF_SKILL_INDICATORS "SkillIndicators"
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

class AbilityParameter final {
public:
	std::string Name;
	std::unordered_map<std::string, std::any> Arguments;
};

class SkillDetail final {
public:
	int32_t Level;
	std::string Description;
	std::vector<AbilityParameter> Abilities;
};

class SkillParameter final {
public:
	int32_t GetMaxLevel() { return MaxLevel; }
	SkillDetail& GetDetail(int32_t level)
	{
		if (Details.size() == 0) return SkillDetail();

		if (level <= 0) return Details.front();

		auto l = level;
		if (l > MaxLevel) l = MaxLevel;
		while (l >= 0) {
			const auto d = std::find_if(Details.begin(), Details.end(), [l](const auto& x) { return x.Level == l; });
			if (d != Details.end()) return *d;
			--l;
		}
		return Details.front();
	}
	std::string GetDescription(int32_t level) { return GetDetail(level).Description; }

public:
	std::string Name;
	std::string IconPath;
	std::vector<SkillDetail> Details;
	int32_t CurrentLevel;
	int32_t MaxLevel;
};

class SkillIndicators final {
private:
	std::vector<SImage*> indicatorIcons;
	mutable CallbackObject* callback;

public:
	SkillIndicators();
	~SkillIndicators();

	uint32_t GetSkillIndicatorCount() const;
	SImage* GetSkillIndicatorImage(uint32_t index);
	void SetCallback(asIScriptFunction* func);
	int AddSkillIndicator(const std::string& icon);
	void TriggerSkillIndicator(int index) const;
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
class Result;

class Ability {
private:
	MethodObject* methodInitialize;
	MethodObject* methodOnStart;
	MethodObject* methodOnFinish;
	MethodObject* methodOnJudge;

private:
	Ability(MethodObject*, MethodObject*, MethodObject*, MethodObject*);
public:
	~Ability();

public:
	static Ability* Create(asIScriptObject*);

	bool Initialize(CScriptDictionary* args, SkillIndicators* indicators);
	bool OnStart(Result* result);
	bool OnFinish(Result* result);
	bool OnJudge(Result* result, JudgeInformation* judgeInfo);
};

void RegisterSkillTypes(asIScriptEngine* engine);
