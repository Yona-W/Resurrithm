#pragma once

#include "ScriptResource.h"
#include <boost/any.hpp>
#include <angelscript.h>

#define SU_IF_ABILITY "Ability"
#define SU_IF_SKILL "Skill"
#define SU_IF_SKILL_MANAGER "SkillManager"
#define SU_IF_SKILL_INDICATORS "SkillIndicators"
#define SU_IF_SKILL_CALLBACK "SkillCallback"
#define SU_IF_NOTETYPE "NoteType"
#define SU_IF_JUDGETYPE "JudgeType"
#define SU_IF_JUDGE_DATA "JudgeData"

class ExecutionManager;
class CallbackObject;

class AbilityParameter final {
public:
    std::string Name;
    std::unordered_map<std::string, boost::any> Arguments;
};

class SkillDetail final {
public:
    int Level;
    std::string Description;
    std::vector<AbilityParameter> Abilities;
};

class SkillParameter final {
public:
    int32_t GetMaxLevel() { return MaxLevel; }
    SkillDetail& GetDetail(int32_t level)
    {
        auto l = level;
        if (l > MaxLevel) l = MaxLevel;
        while (l >= 0) {
            auto d = Details.find(l);
            if (d != Details.end()) return d->second;
            --l;
        }
        return Details.begin()->second;
    }
    std::string GetDescription(int32_t level)
    {
        return GetDetail(level).Description;
    }

public:
    std::string Name;
    std::string IconPath;
    std::map<int32_t, SkillDetail> Details;
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

class SkillManager final {
private:
    std::vector<std::shared_ptr<SkillParameter>> skills;
    int selected;

    void LoadFromToml(boost::filesystem::path file);

public:
    explicit SkillManager();

    void LoadAllSkills();

    void Next();
    void Previous();
    SkillParameter* GetSkillParameter(int relative);
    std::shared_ptr<SkillParameter> GetSkillParameterSafe(int relative);

    int32_t GetSize() const;
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
    void SetCallback(asIScriptFunction *func);
    int AddSkillIndicator(const std::string &icon);
    void TriggerSkillIndicator(int index) const;
};

void RegisterSkillTypes(asIScriptEngine *engine);
