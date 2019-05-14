#pragma once

#include "Skill.h"

enum class NoteAttribute {
	Invisible = 0,
	Finished,
	HellChecking,
	Completed,
	Activated,
};

class SusDrawableNoteData;
class ScoreProcessor {
public:
	virtual ~ScoreProcessor() = default;
	static std::vector<std::shared_ptr<SusDrawableNoteData>> DefaultDataValue;

	virtual void SetJudgeAdjusts(double jas, double jms, double jaa, double jma) = 0;
	virtual void Reset() = 0;
	virtual void Update(std::vector<std::shared_ptr<SusDrawableNoteData>>& notes) = 0;
	virtual void MovePosition(double relative) = 0;
	virtual void Draw() = 0;
	virtual bool ShouldJudge(std::shared_ptr<SusDrawableNoteData> note) = 0;
};

class ScenePlayer;
class ControlState;
class SImage;
class PlayableProcessor : public ScoreProcessor {
protected:
	ScenePlayer* player;
	std::shared_ptr<ControlState> currentState;
	std::vector<std::shared_ptr<SusDrawableNoteData>>& data = DefaultDataValue;
	bool isInHold = false, isInSlide = false, isInAA = false, isInAir = false;
	bool wasInHold = false, wasInSlide = false, wasInAA = false, wasInAir = false;
	SImage* imageHoldLight;
	double judgeWidthAttack;
	double judgeWidthJustice;
	double judgeWidthJusticeCritical;
	double judgeAdjustSlider;
	double judgeAdjustAirString;
	double judgeMultiplierSlider;
	double judgeMultiplierAir;
	bool isAutoAir = false;

	void ProcessScore(const std::shared_ptr<SusDrawableNoteData>& notes);
	bool CheckJudgement(const std::shared_ptr<SusDrawableNoteData>& note) const;
	bool CheckHellJudgement(const std::shared_ptr<SusDrawableNoteData>& note) const;
	bool CheckAirJudgement(const std::shared_ptr<SusDrawableNoteData>& note) const;
	bool CheckAirActionJudgement(const std::shared_ptr<SusDrawableNoteData>& note) const;
	bool CheckHoldJudgement(const std::shared_ptr<SusDrawableNoteData>& note) const;
	bool CheckSlideJudgement(const std::shared_ptr<SusDrawableNoteData>& note) const;
	void IncrementCombo(const std::shared_ptr<SusDrawableNoteData>& note, double reltime, const JudgeInformation& info, const std::string& extra) const;
	void IncrementComboEx(const std::shared_ptr<SusDrawableNoteData>& note, const std::string& extra) const;
	void IncrementComboHell(const std::shared_ptr<SusDrawableNoteData>& note, int state, const std::string& extra) const;
	void IncrementComboAir(const std::shared_ptr<SusDrawableNoteData>& note, double reltime, const JudgeInformation& info, const std::string& extra) const;
	void ResetCombo(const std::shared_ptr<SusDrawableNoteData>& note, const JudgeInformation& info) const;

public:
	PlayableProcessor(ScenePlayer* player);
	PlayableProcessor(ScenePlayer* player, bool autoAir);

	void SetAutoAir(bool flag);
	void SetJudgeWidths(double jc, double j, double a);
	void SetJudgeAdjusts(double jas, double jms, double jaa, double jma) override;
	void Reset() override;
	bool ShouldJudge(std::shared_ptr<SusDrawableNoteData> note) override;
	void Update(std::vector<std::shared_ptr<SusDrawableNoteData>>& notes) override;
	void MovePosition(double relative) override;
	void Draw() override;
};

class AutoPlayerProcessor : public ScoreProcessor {
protected:
	ScenePlayer* player;
	std::vector<std::shared_ptr<SusDrawableNoteData>>& data = DefaultDataValue;
	bool isInHold = false, isInSlide = false, isInAA = false;
	bool wasInHold = false, wasInSlide = false, wasInAA = false;

	void ProcessScore(const std::shared_ptr<SusDrawableNoteData>& notes);
	void IncrementCombo(const JudgeInformation& info, const std::string& extra) const;

public:
	AutoPlayerProcessor(ScenePlayer* player);

	void SetJudgeAdjusts(double jas, double jms, double jaa, double jma) override;
	void Reset() override;
	bool ShouldJudge(std::shared_ptr<SusDrawableNoteData> note) override;
	void Update(std::vector<std::shared_ptr<SusDrawableNoteData>>& notes) override;
	void MovePosition(double relative) override;
	void Draw() override;
};
