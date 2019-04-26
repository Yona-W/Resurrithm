#pragma once

#define SU_IF_DRESULT "DrawableResult"
#define SU_IF_RESULT "Result"

struct DrawableResult {
	uint32_t JusticeCritical;
	uint32_t Justice;
	uint32_t Attack;
	uint32_t Miss;

	uint32_t Combo;
	uint32_t MaxCombo;

	uint32_t Notes;
	uint32_t PastNotes;

	uint32_t FulfilledGauges;
	double CurrentGaugeRatio;
	double RawGaugeValue;
	double GaugeBySkill;

	uint32_t Score;
};

class Result final {
private:
	uint32_t notes = 0;
	uint32_t justiceCritical = 0;
	uint32_t justice = 0;
	uint32_t attack = 0;
	uint32_t miss = 0;
	uint32_t currentCombo = 0;
	uint32_t maxCombo = 0;

	double gaugeValue = 0;
	double gaugePerJusticeCritical = 0;
	double gaugeBoostBySkill = 0;

	double currentScore = 0;
	double scorePerJusticeCritical = 0;

public:
	void SetAllNotes(uint32_t notes);
	void Reset();
	void PerformJusticeCritical();
	void PerformJustice();
	void PerformAttack();
	void PerformMiss();
	void BoostGaugeByValue(double value);
	void BoostGaugeJusticeCritical(double ratio);
	void BoostGaugeJustice(double ratio);
	void BoostGaugeAttack(double ratio);
	void BoostGaugeMiss(double ratio);
	void GetCurrentResult(DrawableResult* result) const;
};

void RegisterResultTypes(asIScriptEngine* engine);
