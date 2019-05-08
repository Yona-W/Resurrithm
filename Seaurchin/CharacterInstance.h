#pragma once

class AngelScript;
class CharacterParameter;
class SkillParameter;
class CharacterImageSet;
class SkillIndicators;
class Result;
class Ability;
class CallbackObject;
class JudgeInformation;
class ScriptScene;

class CharacterInstance final {
private:
	int reference = 0;

	std::shared_ptr<AngelScript> scriptInterface;
	std::shared_ptr<CharacterParameter> characterSource;
	std::shared_ptr<SkillParameter> skillSource;
	CharacterImageSet* imageSet;
	std::shared_ptr<SkillIndicators> indicators;
	std::shared_ptr<Result> targetResult;

	std::vector<Ability*> abilities;
	mutable CallbackObject* judgeCallback;

	void LoadAbilities();
	void CreateImageSet();

	void CallJudgeCallback(const JudgeInformation& info, const std::string& extra) const;

public:
	void AddRef() { reference++; }
	void Release() { if (--reference == 0) delete this; }
	int GetRefCount() const { return reference; }

	CharacterInstance(const std::shared_ptr<CharacterParameter> & character, const std::shared_ptr<SkillParameter> & skill,
		const std::shared_ptr<AngelScript> & script, const std::shared_ptr<Result> & result);
	~CharacterInstance();

	void OnStart();
	void OnFinish();
	void OnJudge(const JudgeInformation & info, const std::string & extra);
	void SetCallback(asIScriptFunction * func, ScriptScene * sceneObj);

	CharacterParameter* GetCharacterParameter() const;
	CharacterImageSet* GetCharacterImages() const;
	SkillParameter* GetSkillParameter() const;
	SkillIndicators* GetSkillIndicators() const;

	static std::shared_ptr<CharacterInstance> CreateInstance(std::shared_ptr<CharacterParameter> character, std::shared_ptr<SkillParameter> skill, std::shared_ptr<AngelScript> script, std::shared_ptr<Result> result);
};

void RegisterCharacterSkillTypes(asIScriptEngine* engine);
