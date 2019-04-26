#include "Misc.h"
#include "Result.h"

void Result::SetAllNotes(const uint32_t hnotes)
{
	notes = hnotes ? hnotes : 1;
	gaugePerJusticeCritical = 60000.0 / notes;
	scorePerJusticeCritical = 1010000.0 / notes;
	Reset();
}

void Result::Reset()
{
	justiceCritical = 0;
	justice = 0;
	attack = 0;
	miss = 0;
	gaugeValue = gaugeBoostBySkill = 0;
	currentScore = 0;
	currentCombo = maxCombo = 0;
}

void Result::PerformJusticeCritical()
{
	justiceCritical++;
	currentCombo++;
	maxCombo = currentCombo > maxCombo ? currentCombo : maxCombo;
	gaugeValue += gaugePerJusticeCritical;
	currentScore += scorePerJusticeCritical;
	gaugeValue = std::max(0.0, gaugeValue);
}

void Result::PerformJustice()
{
	justice++;
	currentCombo++;
	maxCombo = currentCombo > maxCombo ? currentCombo : maxCombo;
	gaugeValue += gaugePerJusticeCritical * 0.8;
	currentScore += scorePerJusticeCritical / 1.01;
	gaugeValue = std::max(0.0, gaugeValue);
}

void Result::PerformAttack()
{
	attack++;
	currentCombo++;
	maxCombo = currentCombo > maxCombo ? currentCombo : maxCombo;
	gaugeValue += gaugePerJusticeCritical * 0.1;
	currentScore += scorePerJusticeCritical / 1.01 * 0.5;
	gaugeValue = std::max(0.0, gaugeValue);
}

void Result::PerformMiss()
{
	miss++;
	currentCombo = 0;
}

void Result::BoostGaugeByValue(double value)
{
	gaugeValue += value;
	gaugeBoostBySkill += value;
	gaugeValue = std::max(0.0, gaugeValue);
}

void Result::BoostGaugeJusticeCritical(const double ratio)
{
	const auto value = gaugePerJusticeCritical * ratio;
	BoostGaugeByValue(value);
}

void Result::BoostGaugeJustice(const double ratio)
{
	const auto value = gaugePerJusticeCritical * ratio * 0.5;
	BoostGaugeByValue(value);
}

void Result::BoostGaugeAttack(const double ratio)
{
	const auto value = gaugePerJusticeCritical * ratio * 0.1;
	BoostGaugeByValue(value);
}

void Result::BoostGaugeMiss(double ratio)
{
	gaugeValue = std::max(0.0, gaugeValue);
}

void Result::GetCurrentResult(DrawableResult* result) const
{
	if (!result) return;
	result->JusticeCritical = justiceCritical;
	result->Justice = justice;
	result->Attack = attack;
	result->Miss = miss;
	result->Combo = currentCombo;
	result->MaxCombo = maxCombo;
	result->Score = SU_TO_INT32(round(currentScore));

	result->Notes = notes;
	result->PastNotes = justiceCritical + justice + attack + miss;

	result->FulfilledGauges = 0;
	result->CurrentGaugeRatio = 0;
	result->RawGaugeValue = gaugeValue;
	result->GaugeBySkill = gaugeBoostBySkill;
	auto cg = round(gaugeValue);
	auto ng = 12000;
	const auto delta = 2000;
	while (cg >= ng) {
		cg -= ng;
		result->FulfilledGauges++;
		ng += delta;
	}
	result->CurrentGaugeRatio = double(cg) / double(ng);
}

void RegisterResultTypes(asIScriptEngine* engine)
{
	engine->RegisterObjectType(SU_IF_DRESULT, sizeof(DrawableResult), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<DrawableResult>());
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 JusticeCritical", asOFFSET(DrawableResult, JusticeCritical));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 Justice", asOFFSET(DrawableResult, Justice));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 Attack", asOFFSET(DrawableResult, Attack));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 Miss", asOFFSET(DrawableResult, Miss));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 Combo", asOFFSET(DrawableResult, Combo));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 MaxCombo", asOFFSET(DrawableResult, MaxCombo));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 Notes", asOFFSET(DrawableResult, Notes));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 PastNotes", asOFFSET(DrawableResult, PastNotes));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 Score", asOFFSET(DrawableResult, Score));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "uint32 FulfilledGauges", asOFFSET(DrawableResult, FulfilledGauges));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "double CurrentGaugeRatio", asOFFSET(DrawableResult, CurrentGaugeRatio));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "double RawGaugeValue", asOFFSET(DrawableResult, RawGaugeValue));
	engine->RegisterObjectProperty(SU_IF_DRESULT, "double GaugeBySkill", asOFFSET(DrawableResult, GaugeBySkill));

	engine->RegisterObjectType(SU_IF_RESULT, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void PerformJusticeCritical()", asMETHOD(Result, PerformJusticeCritical), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void PerformJustice()", asMETHOD(Result, PerformJustice), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void PerformAttack()", asMETHOD(Result, PerformAttack), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void PerformMiss()", asMETHOD(Result, PerformMiss), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void BoostGaugeByValue(double)", asMETHOD(Result, BoostGaugeByValue), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void BoostGaugeJusticeCritical(double)", asMETHOD(Result, BoostGaugeJusticeCritical), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void BoostGaugeJustice(double)", asMETHOD(Result, BoostGaugeJustice), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void BoostGaugeAttack(double)", asMETHOD(Result, BoostGaugeAttack), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void BoostGaugeMiss(double)", asMETHOD(Result, BoostGaugeMiss), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_RESULT, "void GetCurrentResult(" SU_IF_DRESULT " &out)", asMETHOD(Result, GetCurrentResult), asCALL_THISCALL);
}
