#pragma once

#define SU_IF_ABILITY "Ability"
#define SU_IF_SKILL "Skill"
#define SU_IF_SKILL_INDICATORS "SkillIndicators"
#define SU_IF_SKILL_CALLBACK "SkillCallback"
#define SU_IF_NOTETYPE "NoteType"
#define SU_IF_JUDGETYPE "JudgeType"
#define SU_IF_JUDGE_DATA "JudgeData"

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

void RegisterSkillTypes(asIScriptEngine* engine);
