#include "ScenePlayer.h"
#include "ScoreProcessor.h"
#include "ExecutionManager.h"

using namespace std;

// ScoreProcessor-s -------------------------------------------

vector<shared_ptr<SusDrawableNoteData>> ScoreProcessor::DefaultDataValue;

AutoPlayerProcessor::AutoPlayerProcessor(ScenePlayer* splayer)
{
	player = splayer;
}

void AutoPlayerProcessor::SetJudgeAdjusts(const double jas, const double jms, const double jaa, const double jma)
{}

void AutoPlayerProcessor::Reset()
{
	player->currentResult->Reset();
	data = player->data;
	auto an = 0;
	for (auto& note : data) {
		const auto type = note->Type.to_ulong();
		if (type & SU_NOTE_LONG_MASK) {
			if (!note->Type.test(size_t(SusNoteType::AirAction))) an++;
			for (auto& ex : note->ExtraData)
				if (
					ex->Type.test(size_t(SusNoteType::End))
					|| ex->Type.test(size_t(SusNoteType::Step))
					|| ex->Type.test(size_t(SusNoteType::Injection)))
					an++;
		}
		else if (type & SU_NOTE_SHORT_MASK) {
			an++;
		}
	}
	player->currentResult->SetAllNotes(an);
}

bool AutoPlayerProcessor::ShouldJudge(const std::shared_ptr<SusDrawableNoteData> note)
{
	const auto current = player->currentTime - note->StartTime + player->soundBufferingLatency;
	const auto extra = 0.5;
	if (note->Type.to_ulong() & SU_NOTE_LONG_MASK) {
		return current >= -extra && current - note->Duration <= extra;
	}
	if (note->Type.to_ulong() & SU_NOTE_SHORT_MASK) {
		return current >= -extra && current <= extra;
	}
	return false;
}

void AutoPlayerProcessor::Update(vector<shared_ptr<SusDrawableNoteData>> & notes)
{
	auto slideCheck = false;
	auto holdCheck = false;
	auto aaCheck = false;
	for (auto& note : notes) {
		ProcessScore(note);
		slideCheck = isInSlide || slideCheck;
		holdCheck = isInHold || holdCheck;
		aaCheck = isInAA || aaCheck;
	}

	if (!wasInSlide && slideCheck) player->EnqueueJudgeSound(JudgeSoundType::Sliding);
	if (wasInSlide && !slideCheck) player->EnqueueJudgeSound(JudgeSoundType::SlidingStop);
	if (!wasInHold && holdCheck) player->EnqueueJudgeSound(JudgeSoundType::Holding);
	if (wasInHold && !holdCheck) player->EnqueueJudgeSound(JudgeSoundType::HoldingStop);
	if (!wasInAA && aaCheck) player->EnqueueJudgeSound(JudgeSoundType::AirHolding);
	if (wasInAA && !aaCheck) player->EnqueueJudgeSound(JudgeSoundType::AirHoldingStop);
	player->airActionShown = aaCheck;

	wasInHold = holdCheck;
	wasInSlide = slideCheck;
	wasInAA = aaCheck;
}

void AutoPlayerProcessor::MovePosition(const double relative)
{
	const auto newTime = player->currentTime + relative;
	player->currentResult->Reset();

	wasInHold = isInHold = false;
	wasInSlide = isInSlide = false;
	wasInAA = isInAA = false;
	player->EnqueueJudgeSound(JudgeSoundType::HoldingStop);
	player->EnqueueJudgeSound(JudgeSoundType::SlidingStop);
	player->EnqueueJudgeSound(JudgeSoundType::AirHoldingStop);
	player->RemoveSlideEffect();

	// 送り: 飛ばした部分をFinishedに
	// 戻し: 入ってくる部分をUn-Finishedに
	for (auto& note : data) {
		if (note->Type.test(size_t(SusNoteType::Hold))
			|| note->Type.test(size_t(SusNoteType::Slide))
			|| note->Type.test(size_t(SusNoteType::AirAction))) {
			if (note->StartTime <= newTime) {
				note->OnTheFlyData.set(size_t(NoteAttribute::Finished));

				if (!note->Type.test(size_t(SusNoteType::AirAction))) player->currentResult->PerformMiss();
			}
			else {
				note->OnTheFlyData.reset(size_t(NoteAttribute::Finished));
			}
			note->OnTheFlyData.reset(size_t(NoteAttribute::Activated));
			for (auto& extra : note->ExtraData) {
				if (!extra->Type.test(size_t(SusNoteType::End))
					&& !extra->Type.test(size_t(SusNoteType::Step))
					&& !extra->Type.test(size_t(SusNoteType::Injection))) continue;
				if (extra->StartTime <= newTime) {
					extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));

					if (
						extra->Type.test(size_t(SusNoteType::End))
						|| extra->Type.test(size_t(SusNoteType::Step))
						|| extra->Type.test(size_t(SusNoteType::Injection))) player->currentResult->PerformMiss();
				}
				else {
					extra->OnTheFlyData.reset(size_t(NoteAttribute::Finished));
				}
			}
		}
		else {
			if (note->StartTime <= newTime) {
				note->OnTheFlyData.set(size_t(NoteAttribute::Finished));

				if (note->Type.to_ulong() & SU_NOTE_SHORT_MASK) player->currentResult->PerformMiss();
			}
			else {
				note->OnTheFlyData.reset(size_t(NoteAttribute::Finished));
			}
		}
	}
}

void AutoPlayerProcessor::Draw()
{}

void AutoPlayerProcessor::ProcessScore(const shared_ptr<SusDrawableNoteData> & note)
{
	const auto relpos = player->currentTime - note->StartTime + player->soundBufferingLatency;
	if (relpos < 0 || (note->OnTheFlyData.test(size_t(NoteAttribute::Finished)) && note->ExtraData.empty())) return;

	if (note->Type.test(size_t(SusNoteType::Hold))) {
		isInHold = true;
		if (!note->OnTheFlyData.test(size_t(NoteAttribute::Finished))) {
			player->EnqueueJudgeSound(JudgeSoundType::Tap);
			player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
			IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Hold, note->StartLane, note->StartLane + note->Length }, "");
			note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
		}

		note->OnTheFlyData.set(size_t(NoteAttribute::Activated));

		for (auto& extra : note->ExtraData) {
			const auto pos = player->currentTime - extra->StartTime + player->soundBufferingLatency;
			if (pos < 0) continue;
			if (extra->Type.test(size_t(SusNoteType::End))) isInHold = false;
			if (extra->OnTheFlyData.test(size_t(NoteAttribute::Finished))) continue;
			if (extra->Type[size_t(SusNoteType::Injection)]) {
				IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Hold, note->StartLane, note->StartLane + note->Length }, "");
				extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));
				return;
			}
			player->EnqueueJudgeSound(JudgeSoundType::HoldStep);
			player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
			IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Hold, note->StartLane, note->StartLane + note->Length }, "");
			extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));
			return;
		}
	}
	else if (note->Type.test(size_t(SusNoteType::Slide))) {
		isInSlide = true;
		if (!note->OnTheFlyData.test(size_t(NoteAttribute::Finished))) {
			player->EnqueueJudgeSound(JudgeSoundType::Tap);
			player->SpawnSlideLoopEffect(note);

			IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Slide, note->StartLane, note->StartLane + note->Length }, "");
			note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
		}

		note->OnTheFlyData.set(size_t(NoteAttribute::Activated));

		for (auto& extra : note->ExtraData) {
			const auto pos = player->currentTime - extra->StartTime + player->soundBufferingLatency;
			if (pos < 0) continue;
			if (extra->Type.test(size_t(SusNoteType::End))) isInSlide = false;
			if (extra->Type.test(size_t(SusNoteType::Control))) continue;
			if (extra->Type.test(size_t(SusNoteType::Invisible))) continue;
			if (extra->OnTheFlyData.test(size_t(NoteAttribute::Finished))) continue;
			if (extra->Type.test(size_t(SusNoteType::Injection))) {
				IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Slide, extra->StartLane, extra->StartLane + extra->Length }, "");
				extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));
				return;
			}
			player->EnqueueJudgeSound(JudgeSoundType::SlideStep);
			player->SpawnJudgeEffect(extra, JudgeType::SlideTap);
			IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Slide, extra->StartLane, extra->StartLane + extra->Length }, "");
			extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));
			return;
		}
	}
	else if (note->Type.test(size_t(SusNoteType::AirAction))) {
		isInAA = true;

		note->OnTheFlyData.set(size_t(NoteAttribute::Activated));

		for (auto& extra : note->ExtraData) {
			const auto pos = player->currentTime - extra->StartTime + player->soundBufferingLatency;
			if (pos < 0) continue;
			if (extra->Type.test(size_t(SusNoteType::End))) isInAA = false;
			if (extra->Type.test(size_t(SusNoteType::Control))) continue;
			if (extra->Type.test(size_t(SusNoteType::Invisible))) continue;
			if (extra->OnTheFlyData.test(size_t(NoteAttribute::Finished))) continue;
			if (extra->Type[size_t(SusNoteType::Injection)]) {
				IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::AirAction, extra->StartLane, extra->StartLane + extra->Length }, "");
				extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));
				return;
			}
			player->EnqueueJudgeSound(JudgeSoundType::AirAction);
			player->SpawnJudgeEffect(extra, JudgeType::Action);
			IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::AirAction, extra->StartLane, extra->StartLane + extra->Length }, "");
			extra->OnTheFlyData.set(size_t(NoteAttribute::Finished));
		}
	}
	else if (note->Type.test(size_t(SusNoteType::Air))) {
		if (note->Type[size_t(SusNoteType::Up)]) {
			player->EnqueueJudgeSound(JudgeSoundType::Air);
		}
		else {
			player->EnqueueJudgeSound(JudgeSoundType::AirDown);
		}
		player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
		player->SpawnJudgeEffect(note, JudgeType::ShortEx);
		IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Air, note->StartLane, note->StartLane + note->Length }, "");
		note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
	}
	else if (note->Type.test(size_t(SusNoteType::Tap))) {
		player->EnqueueJudgeSound(JudgeSoundType::Tap);
		player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
		IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Tap, note->StartLane, note->StartLane + note->Length }, "");
		note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
	}
	else if (note->Type.test(size_t(SusNoteType::ExTap))) {
		player->EnqueueJudgeSound(JudgeSoundType::ExTap);
		player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
		player->SpawnJudgeEffect(note, JudgeType::ShortEx);
		IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::ExTap, note->StartLane, note->StartLane + note->Length }, "");
		note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
	}
	else if (note->Type.test(size_t(SusNoteType::AwesomeExTap))) {
		player->EnqueueJudgeSound(JudgeSoundType::ExTap);
		player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
		player->SpawnJudgeEffect(note, JudgeType::ShortEx);
		IncrementCombo(
			{ AbilityJudgeType::JusticeCritical, AbilityNoteType::AwesomeExTap, note->StartLane, note->StartLane + note->Length },
			note->Type[size_t(SusNoteType::Down)] ? "AwesomeExTapDown" : "AwesomeExTapUp"
		);
		note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
	}
	else if (note->Type.test(size_t(SusNoteType::Flick))) {
		player->EnqueueJudgeSound(JudgeSoundType::Flick);
		player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
		IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::Flick, note->StartLane, note->StartLane + note->Length }, "");
		note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
	}
	else if (note->Type.test(size_t(SusNoteType::HellTap))) {
		player->EnqueueJudgeSound(JudgeSoundType::Tap);
		player->SpawnJudgeEffect(note, JudgeType::ShortNormal);
		IncrementCombo({ AbilityJudgeType::JusticeCritical, AbilityNoteType::HellTap, note->StartLane, note->StartLane + note->Length }, "");
		note->OnTheFlyData.set(size_t(NoteAttribute::Finished));
	}
}

void AutoPlayerProcessor::IncrementCombo(const JudgeInformation & info, const string & extra) const
{
	player->currentResult->PerformJusticeCritical();
	player->currentCharacterInstance->OnJudge(info, extra);
}
