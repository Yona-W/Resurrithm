#include "ScenePlayer.h"
#include "ScriptSprite.h"
#include "ScriptSpriteMover.h"
#include "ExecutionManager.h"
#include "SettingManager.h"
#include "Setting.h"

using namespace std;

static const uint16_t rectVertexIndices[] = { 0, 1, 3, 3, 1, 2 };
static VERTEX3D groundVertices[] = {
	{ VGet(SU_LANE_X_MIN, SU_LANE_Y_GROUND, SU_LANE_Z_MAX), VGet(0, 1, 0), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.0f, 0.0f, 0.0f, 0.0f },
{ VGet(SU_LANE_X_MAX, SU_LANE_Y_GROUND, SU_LANE_Z_MAX), VGet(0, 1, 0), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 1.0f, 0.0f, 0.0f, 0.0f },
{ VGet(SU_LANE_X_MAX, SU_LANE_Y_GROUND, SU_LANE_Z_MIN_EXT), VGet(0, 1, 0), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 1.0f, 1.0f, 0.0f, 0.0f },
{ VGet(SU_LANE_X_MIN, SU_LANE_Y_GROUND, SU_LANE_Z_MIN_EXT), VGet(0, 1, 0), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.0f, 1.0f, 0.0f, 0.0f }
};


void ScenePlayer::LoadResources()
{
	imageLaneGround = dynamic_cast<SImage*>(resources["LaneGround"]);
	imageLaneJudgeLine = dynamic_cast<SImage*>(resources["LaneJudgeLine"]);
	imageTap = dynamic_cast<SImage*>(resources["Tap"]);
	imageExTap = dynamic_cast<SImage*>(resources["ExTap"]);
	imageFlick = dynamic_cast<SImage*>(resources["Flick"]);
	imageHellTap = dynamic_cast<SImage*>(resources["HellTap"]);
	imageAir = dynamic_cast<SImage*>(resources["Air"]);
	imageAirUp = dynamic_cast<SImage*>(resources["AirUp"]);
	imageAirDown = dynamic_cast<SImage*>(resources["AirDown"]);
	imageHold = dynamic_cast<SImage*>(resources["Hold"]);
	imageHoldStep = dynamic_cast<SImage*>(resources["HoldStep"]);
	imageHoldStrut = dynamic_cast<SImage*>(resources["HoldStrut"]);
	imageSlide = dynamic_cast<SImage*>(resources["Slide"]);
	imageSlideStep = dynamic_cast<SImage*>(resources["SlideStep"]);
	imageSlideStrut = dynamic_cast<SImage*>(resources["SlideStrut"]);
	imageAirAction = dynamic_cast<SImage*>(resources["AirAction"]);
	animeTap = dynamic_cast<SAnimatedImage*>(resources["EffectTap"]);
	animeExTap = dynamic_cast<SAnimatedImage*>(resources["EffectExTap"]);
	animeSlideTap = dynamic_cast<SAnimatedImage*>(resources["EffectSlideTap"]);
	animeSlideLoop = dynamic_cast<SAnimatedImage*>(resources["EffectSlideLoop"]);
	animeAirAction = dynamic_cast<SAnimatedImage*>(resources["EffectAirAction"]);

	soundTap = dynamic_cast<SSound*>(resources["SoundTap"]);
	soundExTap = dynamic_cast<SSound*>(resources["SoundExTap"]);
	soundFlick = dynamic_cast<SSound*>(resources["SoundFlick"]);
	soundAir = dynamic_cast<SSound*>(resources["SoundAir"]);
	soundAirDown = dynamic_cast<SSound*>(resources["SoundAirDown"]);
	soundAirAction = dynamic_cast<SSound*>(resources["SoundAirAction"]);
	soundAirLoop = dynamic_cast<SSound*>(resources["SoundAirHoldLoop"]);
	soundSlideLoop = dynamic_cast<SSound*>(resources["SoundSlideLoop"]);
	soundHoldLoop = dynamic_cast<SSound*>(resources["SoundHoldLoop"]);
	soundSlideStep = dynamic_cast<SSound*>(resources["SoundSlideStep"]);
	soundHoldStep = dynamic_cast<SSound*>(resources["SoundHoldStep"]);
	soundMetronome = dynamic_cast<SSound*>(resources["Metronome"]);

	const auto setting = manager->GetSettingManagerUnsafe()->GetSettingInstanceUnsafe();
	if (soundHoldLoop) soundHoldLoop->SetLoop(true);
	if (soundSlideLoop) soundSlideLoop->SetLoop(true);
	if (soundAirLoop) soundAirLoop->SetLoop(true);
	if (soundTap) soundTap->SetVolume(setting->ReadValue("Sound", "VolumeTap", 1.0));
	if (soundExTap) soundExTap->SetVolume(setting->ReadValue("Sound", "VolumeExTap", 1.0));
	if (soundFlick) soundFlick->SetVolume(setting->ReadValue("Sound", "VolumeFlick", 1.0));
	if (soundAir) soundAir->SetVolume(setting->ReadValue("Sound", "VolumeAir", 1.0));
	if (soundAirDown) soundAirDown->SetVolume(setting->ReadValue("Sound", "VolumeAir", 1.0));
	if (soundAirAction) soundAirAction->SetVolume(setting->ReadValue("Sound", "VolumeAirAction", 1.0));
	if (soundAirLoop) soundAirLoop->SetVolume(setting->ReadValue("Sound", "VolumeAirAction", 1.0));
	if (soundHoldLoop) soundHoldLoop->SetVolume(setting->ReadValue("Sound", "VolumeHold", 1.0));
	if (soundSlideLoop) soundSlideLoop->SetVolume(setting->ReadValue("Sound", "VolumeSlide", 1.0));
	if (soundHoldStep) soundHoldStep->SetVolume(setting->ReadValue("Sound", "VolumeHold", 1.0));
	if (soundSlideStep) soundSlideStep->SetVolume(setting->ReadValue("Sound", "VolumeSlide", 1.0));
	if (soundMetronome) {
		soundMetronome->SetVolume(setting->ReadValue("Sound", "VolumeTap", 1.0));
	}
	else {
		soundMetronome = soundTap;
	}

	vector<toml::Value> scv = { 0, 200, 255 };
	vector<toml::Value> aajcv = { 128, 255, 160 };
	scv = setting->ReadValue("Play", "ColorSlideLine", scv);
	aajcv = setting->ReadValue("Play", "ColorAirActionJudgeLine", aajcv);
	showSlideLine = setting->ReadValue("Play", "ShowSlideLine", true);
	slideLineThickness = setting->ReadValue("Play", "SlideLineThickness", 16.0) / 2.0;
	showAirActionJudge = setting->ReadValue("Play", "ShowAirActionJudgeLine", true);
	slideLineColor = GetColor(scv[0].as<int>(), scv[1].as<int>(), scv[2].as<int>());
	airActionJudgeColor = GetColor(aajcv[0].as<int>(), aajcv[1].as<int>(), aajcv[2].as<int>());

	// 2^x制限があるのでここで計算
	const auto exty = laneBufferX * SU_LANE_ASPECT_EXT;
	auto bufferY = 2.0f;
	while (exty > bufferY) bufferY *= 2.0f;
	const auto bufferV = SU_TO_FLOAT(exty / bufferY);
	for (auto i = 2; i < 4; i++) groundVertices[i].v = bufferV;
	if (hGroundBuffer) DeleteGraph(hGroundBuffer);
	hGroundBuffer = MakeScreen(SU_TO_INT32(laneBufferX), SU_TO_INT32(bufferY), TRUE);

	if (!spriteLane) {
		SSynthSprite* pSynthSprite = SSynthSprite::Factory(1024, 4224);

		if (imageLaneGround) {
			imageLaneGround->AddRef();
			pSynthSprite->Transfer(imageLaneGround, 0.0, 0.0);
		}
		if (imageLaneJudgeLine) {
			imageLaneJudgeLine->AddRef();
			pSynthSprite->Transfer(imageLaneJudgeLine, 0.0, laneBufferY);
		}

		SShape* pShape = SShape::Factory();
		pShape->Type = SShapeType::BoxFill;
		pShape->SetColor(255, 255, 255);
		pShape->SetWidth(1);
		pShape->SetHeight(laneBufferY * cullingLimit);
		pShape->SetPosY(laneBufferY * cullingLimit / 2.0);
		const auto division = 8;
		for (auto i = 1; i < division; i++) {
			pShape->SetPosX(laneBufferX / division * i);
			pShape->AddRef();
			pSynthSprite->Transfer(pShape);
		}
		pShape->Release();

		spriteLane = pSynthSprite;
	}
}

void ScenePlayer::AddSprite(SSprite * sprite)
{
	spritesPending.push_back(sprite);
}

void ScenePlayer::TickGraphics(const double delta)
{
	UpdateSlideEffect();
}

void ScenePlayer::Draw()
{
	if (movieBackground) DrawExtendGraph(0, 0, SU_RES_WIDTH, SU_RES_HEIGHT, movieBackground, FALSE);

	BEGIN_DRAW_TRANSACTION(hGroundBuffer);
	ClearDrawScreen();

	// 背景部
	spriteLane->Draw();
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	SetDrawBright(255, 255, 255);
	for (auto& note : seenData) {
		auto& type = note->Type;
		if (type[size_t(SusNoteType::MeasureLine)]) DrawMeasureLine(note);
	}

	// 下側のロングノーツ類
	for (auto& note : seenData) {
		auto& type = note->Type;
		if (type[size_t(SusNoteType::Hold)]) DrawHoldNotes(note);
		if (type[size_t(SusNoteType::Slide)]) DrawSlideNotes(note);
	}

	// 上側のショートノーツ類
	for (auto& note : seenData) {
		const auto type = note->Type.to_ulong();
#define GET_BIT(num) (1UL << (int(num)))
		const auto mask = GET_BIT(SusNoteType::Tap) | GET_BIT(SusNoteType::ExTap) | GET_BIT(SusNoteType::AwesomeExTap) | GET_BIT(SusNoteType::Flick) | GET_BIT(SusNoteType::HellTap) | GET_BIT(SusNoteType::Grounded);
#undef GET_BIT
		if (type & mask) DrawShortNotes(note);
	}

	FINISH_DRAW_TRANSACTION;
	Prepare3DDrawCall();
	DrawPolygonIndexed3D(groundVertices, 4, rectVertexIndices, 2, hGroundBuffer, TRUE);
	for (auto& i : sprites) i->Draw();

	//3D系ノーツ
	Prepare3DDrawCall();
	DrawAerialNotes(seenData);

	if (airActionShown && showAirActionJudge) {
		SetDrawBlendMode(DX_BLENDMODE_ADD, 192);
		SetUseZBuffer3D(FALSE);
		DrawTriangle3D(
			VGet(SU_LANE_X_MIN_EXT, SU_LANE_Y_AIR - 5, SU_LANE_Z_MIN - 5),
			VGet(SU_LANE_X_MIN_EXT, SU_LANE_Y_AIR + 5, SU_LANE_Z_MIN + 5),
			VGet(SU_LANE_X_MAX_EXT, SU_LANE_Y_AIR + 5, SU_LANE_Z_MIN + 5),
			airActionJudgeColor, TRUE);
		DrawTriangle3D(
			VGet(SU_LANE_X_MIN_EXT, SU_LANE_Y_AIR - 5, SU_LANE_Z_MIN - 5),
			VGet(SU_LANE_X_MAX_EXT, SU_LANE_Y_AIR + 5, SU_LANE_Z_MIN + 5),
			VGet(SU_LANE_X_MAX_EXT, SU_LANE_Y_AIR - 5, SU_LANE_Z_MIN - 5),
			airActionJudgeColor, TRUE);
	}
}

void ScenePlayer::DrawAerialNotes(const vector<shared_ptr<SusDrawableNoteData>> & notes)
{
	vector<AirDrawQuery> airdraws, covers;
	for (const auto& note : seenData) {
		if (note->Type.test(size_t(SusNoteType::AirAction))) {
			// DrawAirActionNotes(note);
			AirDrawQuery head;
			head.Type = AirDrawType::AirActionStart;
			head.Z = 1.0 - note->ModifiedPosition / seenDuration;
			head.Note = note;
			airdraws.push_back(head);
			auto prev = note;
			auto lastZ = head.Z;
			for (const auto& extra : note->ExtraData) {
				if (extra->Type.test(size_t(SusNoteType::Control))) continue;
				if (extra->Type.test(size_t(SusNoteType::Injection))) continue;
				const auto z = 1.0 - extra->ModifiedPosition / seenDuration;
				if ((z >= 0 || lastZ >= 0) && (z < cullingLimit || lastZ < cullingLimit)) {
					AirDrawQuery tail;
					tail.Type = AirDrawType::AirActionStep;
					tail.Z = z;
					tail.Note = extra;
					tail.PreviousNote = prev;
					airdraws.push_back(tail);
					covers.push_back(tail);
				}
				prev = extra;
				lastZ = z;
			}
		}
		if (note->Type.test(size_t(SusNoteType::Air))) {
			const auto z = 1.0 - note->ModifiedPosition / seenDuration;
			if (z < 0 || z >= cullingLimit) continue;
			AirDrawQuery head;
			head.Type = AirDrawType::Air;
			head.Z = z;
			head.Note = note;
			airdraws.push_back(head);
		}
	}
	stable_sort(airdraws.begin(), airdraws.end(), [](const AirDrawQuery a, const AirDrawQuery b) {
		return a.Z < b.Z;
		});
	for (const auto& query : airdraws) {
		switch (query.Type) {
		case AirDrawType::Air:
			DrawAirNotes(query);
			break;
		case AirDrawType::AirActionStart:
			DrawAirActionStart(query);
			break;
		case AirDrawType::AirActionStep:
			DrawAirActionStep(query);
			break;
		default: break;
		}
	}
	for (const auto& query : covers) DrawAirActionCover(query);
	for (const auto& query : airdraws) {
		if (query.Type == AirDrawType::AirActionStep) DrawAirActionStepBox(query);
	}
}

// position は 0 ~ 16
void ScenePlayer::SpawnJudgeEffect(const shared_ptr<SusDrawableNoteData> & target, const JudgeType type)
{
	Prepare3DDrawCall();
	const auto position = target->StartLane + target->Length / 2.0f;
	const auto x = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, position / 16.0f);
	switch (type) {
	case JudgeType::ShortNormal: {
		const auto spawnAt = ConvWorldPosToScreenPos(VGet(x, SU_LANE_Y_GROUND, SU_LANE_Z_MIN));
		animeTap->AddRef();
		auto sp = SAnimeSprite::Factory(animeTap);
		sp->Apply("origX:128, origY:224");
		sp->Transform.X = spawnAt.x;
		sp->Transform.Y = spawnAt.y;
		{
			sp->AddRef();
			AddSprite(sp);
		}

		sp->Release();
		break;
	}
	case JudgeType::ShortEx: {
		const auto spawnAt = ConvWorldPosToScreenPos(VGet(x, SU_LANE_Y_GROUND, SU_LANE_Z_MIN));
		animeExTap->AddRef();
		auto sp = SAnimeSprite::Factory(animeExTap);
		sp->Apply("origX:128, origY:256");
		sp->Transform.X = spawnAt.x;
		sp->Transform.Y = spawnAt.y;
		sp->Transform.ScaleX = target->Length / 6.0f;
		{
			sp->AddRef();
			AddSprite(sp);
		}

		sp->Release();
		break;
	}
	case JudgeType::SlideTap: {
		const auto spawnAt = ConvWorldPosToScreenPos(VGet(x, SU_LANE_Y_GROUND, SU_LANE_Z_MIN));
		animeSlideTap->AddRef();
		auto sp = SAnimeSprite::Factory(animeSlideTap);
		sp->Apply("origX:128, origY:224");
		sp->Transform.X = spawnAt.x;
		sp->Transform.Y = spawnAt.y;
		{
			sp->AddRef();
			AddSprite(sp);
		}

		sp->Release();
		break;
	}
	case JudgeType::Action: {
		const auto spawnAt = ConvWorldPosToScreenPos(VGet(x, SU_LANE_Y_AIR, SU_LANE_Z_MIN));
		animeAirAction->AddRef();
		auto sp = SAnimeSprite::Factory(animeAirAction);
		sp->Apply("origX:128, origY:128");
		sp->Transform.X = spawnAt.x;
		sp->Transform.Y = spawnAt.y;
		{
			sp->AddRef();
			AddSprite(sp);
		}

		sp->Release();
		break;
	}
	}
}

void ScenePlayer::SpawnSlideLoopEffect(const shared_ptr<SusDrawableNoteData> & target)
{
	SpawnJudgeEffect(target, JudgeType::SlideTap);

	animeSlideLoop->AddRef();
	auto loopefx = SAnimeSprite::Factory(animeSlideLoop);
	loopefx->Apply("origX:128, origY:224");
	loopefx->SetLoopCount(-1);
	slideEffects[target] = loopefx;
	{
		loopefx->AddRef();
		AddSprite(loopefx);
	}
}

void ScenePlayer::RemoveSlideEffect()
{
	auto it = slideEffects.begin();
	while (it != slideEffects.end()) {
		auto effect = (*it).second;
		effect->Dismiss();
		effect->Release();
		it = slideEffects.erase(it);
	}
}

void ScenePlayer::UpdateSlideEffect()
{
	Prepare3DDrawCall();
	auto it = slideEffects.begin();
	while (it != slideEffects.end()) {
		auto note = (*it).first;
		auto effect = (*it).second;
		if (currentTime >= note->StartTime + note->Duration) {
			effect->Dismiss();
			effect->Release();
			it = slideEffects.erase(it);
			continue;
		}
		auto last = note;

		for (auto& slideElement : note->ExtraData) {
			if (slideElement->Type.test(size_t(SusNoteType::Control))) continue;
			if (slideElement->Type.test(size_t(SusNoteType::Injection))) continue;
			if (currentTime >= slideElement->StartTime) {
				last = slideElement;
				continue;
			}
			auto& segmentPositions = curveData[slideElement];

			auto lastSegmentPosition = segmentPositions[0];
			auto lastTimeInBlock = get<0>(lastSegmentPosition) / (slideElement->StartTime - last->StartTime);
			auto comp = false;
			for (auto& segmentPosition : segmentPositions) {
				if (lastSegmentPosition == segmentPosition) continue;
				const auto currentTimeInBlock = get<0>(segmentPosition) / (slideElement->StartTime - last->StartTime);
				const auto cst = glm::mix(last->StartTime, slideElement->StartTime, currentTimeInBlock);
				if (currentTime >= cst) {
					lastSegmentPosition = segmentPosition;
					lastTimeInBlock = currentTimeInBlock;
					continue;
				}
				const auto lst = glm::mix(last->StartTime, slideElement->StartTime, lastTimeInBlock);
				const auto t = (currentTime - lst) / (cst - lst);
				const auto x = glm::mix(get<1>(lastSegmentPosition), get<1>(segmentPosition), t);
				const auto absx = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, x);
				const auto at = ConvWorldPosToScreenPos(VGet(absx, SU_LANE_Y_GROUND, SU_LANE_Z_MIN));
				effect->Transform.X = at.x;
				effect->Transform.Y = at.y;
				comp = true;
				break;
			}
			if (comp) break;
		}
		++it;
	}
}


void ScenePlayer::DrawShortNotes(const shared_ptr<SusDrawableNoteData> & note) const
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	const auto relpos = 1.0 - note->ModifiedPosition / seenDuration;
	const auto length = note->Length;
#ifdef SU_ENABLE_NOTE_HORIZONTAL_MOVING
	const auto zlane = note->CenterAtZero - length / 2.0f;
	const auto slane = glm::mix(zlane, float(note->StartLane), relpos);
#else
	const auto slane = float(note->StartLane);
#endif
	SImage* handleToDraw = nullptr;

	const auto& type = note->Type;
	if (type[size_t(SusNoteType::Tap)]) {
		handleToDraw = imageTap;
	}
	else if (type[size_t(SusNoteType::ExTap)] || type[size_t(SusNoteType::AwesomeExTap)]) {
		handleToDraw = imageExTap;
	}
	else if (type[size_t(SusNoteType::Flick)]) {
		handleToDraw = imageFlick;
	}
	else if (type[size_t(SusNoteType::HellTap)]) {
		handleToDraw = imageHellTap;
	}
	else if (type[size_t(SusNoteType::Air)] && type[size_t(SusNoteType::Grounded)]) {
		handleToDraw = imageAir;
	}

	//64*3 x 64 を描画するから1/2でやる必要がある

	if (handleToDraw) DrawTap(slane, SU_TO_INT32(length), relpos, handleToDraw->GetHandle());
}

void ScenePlayer::DrawAirNotes(const AirDrawQuery & query) const
{
	auto note = query.Note;
	const auto length = note->Length;
	const auto slane = note->StartLane;
	const auto z = glm::mix(SU_LANE_Z_MAX, SU_LANE_Z_MIN, query.Z);
	const auto left = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, slane / 16.0);
	const auto right = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, (slane + length) / 16.0);
	double refroll;
	if (note->ExtraAttribute->RollTimeline) {
		auto state = note->ExtraAttribute->RollTimeline->GetRawDrawStateAt(currentTime);
		const auto mp = note->StartTimeEx - get<1>(state);
		refroll = NormalizedFmod(-mp * airRollSpeed, 0.5);
	}
	else {
		refroll = NormalizedFmod(-note->ModifiedPosition * airRollSpeed, 0.5);
	}
	const auto roll = SU_TO_FLOAT(note->Type.test(size_t(SusNoteType::Up)) ? refroll : 0.5 - refroll);
	const auto xadjust = note->Type.test(size_t(SusNoteType::Left)) ? -80.0f : (note->Type.test(size_t(SusNoteType::Right)) ? 80.0f : 0.0f);
	const auto handle = note->Type.test(size_t(SusNoteType::Up)) ? imageAirUp->GetHandle() : imageAirDown->GetHandle();

	VERTEX3D vertices[] = {
		{
			VGet(left + xadjust, SU_LANE_Y_AIRINDICATE, z),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			0.0f, roll, 0.0f, 0.0f
		},
		{
			VGet(right + xadjust, SU_LANE_Y_AIRINDICATE, z),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			1.0f, roll, 0.0f, 0.0f
		},
		{
			VGet(right, SU_LANE_Y_GROUND, z),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			1.0f, roll + 0.5f, 0.0f, 0.0f
		},
		{
			VGet(left, SU_LANE_Y_GROUND, z),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			0.0f, roll + 0.5f, 0.0f, 0.0f
		}
	};
	Prepare3DDrawCall();
	SetUseZBuffer3D(FALSE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	DrawPolygonIndexed3D(vertices, 4, rectVertexIndices, 2, handle, TRUE);
}

void ScenePlayer::DrawHoldNotes(const shared_ptr<SusDrawableNoteData> & note) const
{
	const auto length = note->Length;
	const auto slane = note->StartLane;
	const auto endpoint = note->ExtraData.back();
	const auto relpos = 1.0 - note->ModifiedPosition / seenDuration;
	const auto reltailpos = 1.0 - endpoint->ModifiedPosition / seenDuration;
	const auto begin = !!note->OnTheFlyData[size_t(NoteAttribute::Finished)]; // Hold全体の判定が行われ始めていればtrueにしたい、これだと判定としては少し遅いかもしれないがまぁ実用上問題ないのでは
	const auto activated = !!note->OnTheFlyData[size_t(NoteAttribute::Activated)]; // Holdが押されていればtrueにしたい、たぶん一致した論理になるはず

	// 中身だけ先に描画
	// 分割しないで描画すべき矩形領域計算してしまえばいいんじゃないでしょうか
	auto head = relpos;
	auto tail = reltailpos;
	if (!(head < 0 && tail < 0) && !(head >= cullingLimit && tail >= cullingLimit)) {
		if (!begin) { // 判定前
			SetDrawBlendMode(DX_BLENDMODE_ADD, 239);
		}
		else if (activated) { // 判定中 : Hold時
			SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
		}
		else { // 判定中 : 非Hold時
			SetDrawBlendMode(DX_BLENDMODE_ADD, 175);
		}

		if (begin && activated) {
			if (head > 1) head = 1;
			if (tail > 1) tail = 1;
		}

		const auto wholelen = fabs(reltailpos - relpos);
		const auto len = fabs(tail - head);

		const auto y1 = SU_TO_FLOAT(laneBufferY * head);
		const auto y2 = SU_TO_FLOAT(laneBufferY * tail);
		const auto srcY = SU_TO_INT32((relpos - head) / wholelen * imageHoldStrut->GetHeight());
		const auto height = SU_TO_INT32(len / wholelen * imageHoldStrut->GetHeight());

		DrawRectModiGraphF(
			slane * widthPerLane, y1,
			(slane + length) * widthPerLane, y1,
			(slane + length) * widthPerLane, y2,
			slane * widthPerLane, y2,
			0, srcY, SU_TO_INT32(noteImageBlockX), height,
			imageHoldStrut->GetHandle(), TRUE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	for (int i = note->ExtraData.size() - 1; i >= 0; --i) {
		const auto& ex = note->ExtraData[i];

		if (ex->Type.test(size_t(SusNoteType::Injection))) continue;
		if (ex->OnTheFlyData[size_t(NoteAttribute::Finished)]/* && ノーツがAttack以上の判定*/) continue;

		const auto relendpos = 1.0 - ex->ModifiedPosition / seenDuration;
		const int len = SU_TO_INT32(length);
		if (ex->Type.test(size_t(SusNoteType::Start))) {
			DrawTap(slane, len, relendpos, imageHold->GetHandle());
		}
		else {
			DrawTap(slane, len, relendpos, imageHoldStep->GetHandle());
		}
	}
	if (!(note->OnTheFlyData[size_t(NoteAttribute::Finished)]/* && ノーツがAttack以上の判定*/)) {
		const int len = SU_TO_INT32(length);
		DrawTap(slane, len, relpos, imageHold->GetHandle());
	}
}

void ScenePlayer::DrawSlideNotes(const shared_ptr<SusDrawableNoteData> & note)
{
	auto lastStep = note;
	auto offsetTimeInBlock = 0.0; /* そのslideElementの、不可視中継点のつながり等を考慮した時の先頭位置、的な */
	const auto strutBottom = 1.0;
	const auto begin = !!note->OnTheFlyData[size_t(NoteAttribute::Finished)]; // Hold全体の判定が行われ始めていればtrueにしたい、これだと判定としては少し遅いかもしれないがまぁ実用上問題ないのでは
	const auto activated = !!note->OnTheFlyData[size_t(NoteAttribute::Activated)]; // Holdが押されていればtrueにしたい、たぶん一致した論理になるはず
	slideVertices.clear();
	slideIndices.clear();

	/* 基本方針 */
	/* 従来 : [始点,中継点,不可視中継点]から次の[中継点,不可視中継点,終点]にかけて(u,v)の計算を行っている */
	/* 改良 : [始点,中継点]から次の[中継点,終点]にかけて、”不可視中継点を超えて”(u,v)の計算を行う */
	/*        分割点(?)の配置はすでに正しく計算されているので手を加えない */
	/*        従来の(u,v)計算を行っていた領域で、 0 <= v <= 1 であったところを a <= v <= b に変更し */
	/*        不可視中継点をまたいだ領域全体の時間からa,bを適当に定める */

	/* 重要 */
	/* すべての変数、演算の意味を理解したわけではないので、変拍子、ハイスピ設定等で死ぬ可能性が多分にある */

	/* 各SlideElementに対応する追加情報を計算して格納する */
	/* 起点時刻, 終点時刻 の2要素ベクターのベクター */
	/* 起点時刻 : そのSlideElement以前に現れた始点or中継点の先頭時刻 */
	/* 終点時刻 : そのSlideElement以降に現れる終点or中継点の終端時刻 */
	std::vector<std::vector<double>> exData(note->ExtraData.size() + 1);
	{
		unsigned int i = 0;
		std::vector<double> tmp(2);
		auto lastStartTime = note->StartTime;

		/* 先頭要素(note)はSusNoteType::Startになるはず(本当か?) */
		tmp[0] = note->StartTime;
		tmp[1] = 0;
		exData[i] = tmp;
		++i;

		/* 直前のSlideElementの開始時刻を共有する */
		/* 中継点の場合は開始時刻を共有した”後”に、共有する開始時刻を更新する */
		/* 中継点、終点の場合は終了時刻が決定するので保存しておく(それ以外なら終端時刻はとりあえず0にしておく) */
		for (auto& slideElement : note->ExtraData) {
			tmp[0] = lastStartTime;
			tmp[1] = 0;

			if (slideElement->Type.test(size_t(SusNoteType::Step))) {
				lastStartTime = slideElement->StartTime;
				tmp[1] = slideElement->StartTime;
			}
			if (slideElement->Type.test(size_t(SusNoteType::End))) {
				/* これSlide終端の後にデータ来ない前提になってるけど(大丈夫か?) */
				tmp[1] = slideElement->StartTime;
			}

			exData[i] = tmp;
			++i;
		}

		for (i = exData.size() - 1; i > 0; --i) {
			/* 自分自身の直前の何かが開始時刻を共有している */
			if (exData[i - 1][0] == exData[i][0]) {
				/* 終了時刻も共有したい */
				exData[i - 1][1] = exData[i][1];
			}
			else {
				/* 終了時刻は既にexData[i - 1][1]に入っているはず(本当か?) */
			}
		}
	}

	// 支柱
	auto drawcount = 0;
	uint16_t base = 0;
	unsigned int i = 0;
	for (auto& slideElement : note->ExtraData) {
		++i; /* exData[0] はnoteそのものの情報だからこのインクリメントは必須 */
		if (slideElement->Type.test(size_t(SusNoteType::Control))) continue;
		if (slideElement->Type.test(size_t(SusNoteType::Injection))) continue;
		auto& segmentPositions = curveData[slideElement];

		auto lastSegmentPosition = segmentPositions[0];
		auto lastSegmentLength = double(lastStep->Length);
		auto lastSegmentRelativeY = 1.0 - lastStep->ModifiedPosition / seenDuration;
		auto lsRelY = lastSegmentRelativeY;
		auto lastTimeInBlock2 = get<0>(lastSegmentPosition) / (exData[i][1] - exData[i][0]);

		for (const auto& segmentPosition : segmentPositions) {
			if (lastSegmentPosition == segmentPosition) continue;
			const auto currentTimeInBlock = get<0>(segmentPosition) / (slideElement->StartTime - lastStep->StartTime);
			const auto currentTimeInBlock2 = get<0>(segmentPosition) / (exData[i][1] - exData[i][0]);
			const auto currentSegmentLength = glm::mix(double(lastStep->Length), double(slideElement->Length), currentTimeInBlock);
			const auto segmentExPosition = glm::mix(lastStep->ModifiedPosition, slideElement->ModifiedPosition, currentTimeInBlock);
			const auto currentSegmentRelativeY = 1.0 - segmentExPosition / seenDuration;
			auto csRelY = currentSegmentRelativeY;
			if ((currentSegmentRelativeY >= 0 || lastSegmentRelativeY >= 0)
				&& (currentSegmentRelativeY < cullingLimit || lastSegmentRelativeY < cullingLimit)) {
				auto currentTimeDiff = 0.0;
				const auto lastTimeDiff = 0.0;

				if (begin && activated) {
					if (csRelY >= 1 && lsRelY >= 1) {
						// セグメントの全体が判定ラインを超えているとき
						// 表示はしたくないけど内部数値は普通に処理した時と一致させたい
						//    => 描画先座標を一致させてお茶を濁す
						lsRelY = csRelY;
					}
					else if (csRelY >= 1) {
						// セグメントの始点が判定ラインより手前、終点が判定ラインを超えているとき
						// 始点はそのまま、終点は判定ラインに一致させ、次の始点は判定ラインから
						csRelY = 1;

						const auto sep = (1.0 - csRelY) * seenDuration;
						const auto ctib = (sep - lastStep->ModifiedPosition) / (slideElement->ModifiedPosition - lastStep->ModifiedPosition);
						const auto g0Sp = ctib * (slideElement->StartTime - lastStep->StartTime);
						const auto ctib2 = g0Sp / (exData[i][1] - exData[i][0]);

						currentTimeDiff = ctib2 - currentTimeInBlock2;
					}
					else if (lsRelY >= 1) {
						// セグメントの終点は判定ラインより手前、始点が判定ラインを超えているとき(ハイスピ指定を行った場合に起こりうるはず)
						// どうしたいんだろう
						const auto ratio = (1.0 - csRelY) / (lsRelY - csRelY);
						lastTimeInBlock2 = currentTimeInBlock2 + (lastTimeInBlock2 - currentTimeInBlock2) * ratio;
						lsRelY = 1;
					}
				}


				slideVertices.push_back(
					{
						VGet(SU_TO_FLOAT(get<1>(lastSegmentPosition) * laneBufferX - lastSegmentLength / 2 * widthPerLane), SU_TO_FLOAT(laneBufferY * lsRelY), 0),
						1.0f,
						GetColorU8(255, 255, 255, 255),
						0.0f, float((offsetTimeInBlock + lastTimeInBlock2 + lastTimeDiff) * strutBottom)
					}
				);
				slideVertices.push_back(
					{
						VGet(SU_TO_FLOAT(get<1>(lastSegmentPosition) * laneBufferX + lastSegmentLength / 2 * widthPerLane), SU_TO_FLOAT(laneBufferY * lsRelY), 0),
						1.0f,
						GetColorU8(255, 255, 255, 255),
						1.0f, float((offsetTimeInBlock + lastTimeInBlock2 + lastTimeDiff) * strutBottom)
					}
				);
				slideVertices.push_back(
					{
						VGet(SU_TO_FLOAT(get<1>(segmentPosition) * laneBufferX - currentSegmentLength / 2 * widthPerLane), SU_TO_FLOAT(laneBufferY * csRelY), 0),
						1.0f,
						GetColorU8(255, 255, 255, 255),
						0.0f, float((offsetTimeInBlock + currentTimeInBlock2 + currentTimeDiff) * strutBottom)
					}
				);
				slideVertices.push_back(
					{
						VGet(SU_TO_FLOAT(get<1>(segmentPosition) * laneBufferX + currentSegmentLength / 2 * widthPerLane), SU_TO_FLOAT(laneBufferY * csRelY), 0),
						1.0f,
						GetColorU8(255, 255, 255, 255),
						1.0f, float((offsetTimeInBlock + currentTimeInBlock2 + currentTimeDiff) * strutBottom)
					}
				);
				vector<uint16_t> here = { base, uint16_t(base + 2), uint16_t(base + 1), uint16_t(base + 2), uint16_t(base + 1), uint16_t(base + 3) };
				slideIndices.insert(slideIndices.end(), here.begin(), here.end());
				base += 4;
				drawcount += 2;
			}
			lastSegmentPosition = segmentPosition;
			lastSegmentLength = currentSegmentLength;
			lastSegmentRelativeY = currentSegmentRelativeY;
			lsRelY = csRelY;
			lastTimeInBlock2 = currentTimeInBlock2;
		}
		if (slideElement->Type.test(size_t(SusNoteType::Step))) {
			offsetTimeInBlock = 0;
		}
		else {
			offsetTimeInBlock += lastTimeInBlock2;
		}
		lastStep = slideElement;
	}

	if (!begin) { // 判定前
		SetDrawBlendMode(DX_BLENDMODE_ADD, 239);
	}
	else if (activated) { // 判定中 : Slide時
		SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
	}
	else { // 判定中 : 非Slide時
		SetDrawBlendMode(DX_BLENDMODE_ADD, 175);
	}

	SetUseBackCulling(FALSE);
	DrawPolygonIndexed2D(slideVertices.data(), slideVertices.size(), slideIndices.data(), drawcount, imageSlideStrut->GetHandle(), TRUE);

	// 中心線
	if (showSlideLine) {
		if (!begin) { // 判定前
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 239);
		}
		else if (activated) { // 判定中 : Hold時
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		}
		else { // 判定中 : 非Hold時
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 175);
		}

		lastStep = note;
		for (auto& slideElement : note->ExtraData) {
			if (slideElement->Type.test(size_t(SusNoteType::Control))) continue;
			if (slideElement->Type.test(size_t(SusNoteType::Injection))) continue;
			auto& segmentPositions = curveData[slideElement];
			auto lastSegmentPosition = segmentPositions[0];
			auto lastSegmentRelativeX = get<1>(lastSegmentPosition);
			auto lastSegmentRelativeY = 1.0 - lastStep->ModifiedPosition / seenDuration;

			for (auto& segmentPosition : segmentPositions) {
				if (lastSegmentPosition == segmentPosition) continue;
				const auto currentTimeInBlock = get<0>(segmentPosition) / (slideElement->StartTime - lastStep->StartTime);
				const auto segmentExPosition = glm::mix(lastStep->ModifiedPosition, slideElement->ModifiedPosition, currentTimeInBlock);
				auto currentSegmentRelativeX = get<1>(segmentPosition);
				auto currentSegmentRelativeY = 1.0 - segmentExPosition / seenDuration;
				if ((currentSegmentRelativeY >= 0 || lastSegmentRelativeY >= 0)
					&& (currentSegmentRelativeY < cullingLimit || lastSegmentRelativeY < cullingLimit)) {
					if (begin && activated) {
						if (currentSegmentRelativeY >= 1 && lastSegmentRelativeY >= 1) {
							// セグメントの全体が判定ラインを超えているとき
							// 表示はしたくないけど内部数値は普通に処理した時と一致させたい
							//    => 描画先座標を一致させてお茶を濁す
							lastSegmentRelativeX = currentSegmentRelativeX;
							lastSegmentRelativeY = currentSegmentRelativeY;
						}
						else if (currentSegmentRelativeY >= 1) {
							// セグメントの始点が判定ラインより手前、終点が判定ラインを超えているとき
							// 始点はそのまま、終点は判定ラインに一致させ、次の始点は判定ラインから
							currentSegmentRelativeX = lastSegmentRelativeX - (lastSegmentRelativeX - currentSegmentRelativeX) / (lastSegmentRelativeY - currentSegmentRelativeY) * (lastSegmentRelativeY - 1.0);
							currentSegmentRelativeY = 1;

						}
						else if (lastSegmentRelativeY >= 1) {
							// セグメントの終点は判定ラインより手前、始点が判定ラインを超えているとき(ハイスピ指定を行った場合に起こりうるはず)
							// どうしたいんだろう
							lastSegmentRelativeX = currentSegmentRelativeX - (currentSegmentRelativeX - lastSegmentRelativeX) / (currentSegmentRelativeY - lastSegmentRelativeY) * (currentSegmentRelativeY - 1.0);
							lastSegmentRelativeY = 1;
						}
					}

					DrawTriangleAA(
						SU_TO_FLOAT(lastSegmentRelativeX * laneBufferX - slideLineThickness), SU_TO_FLOAT(laneBufferY * lastSegmentRelativeY),
						SU_TO_FLOAT(lastSegmentRelativeX * laneBufferX + slideLineThickness), SU_TO_FLOAT(laneBufferY * lastSegmentRelativeY),
						SU_TO_FLOAT(currentSegmentRelativeX * laneBufferX - slideLineThickness), SU_TO_FLOAT(laneBufferY * currentSegmentRelativeY),
						slideLineColor, 16
					);
					DrawTriangleAA(
						SU_TO_FLOAT(lastSegmentRelativeX * laneBufferX + slideLineThickness), SU_TO_FLOAT(laneBufferY * lastSegmentRelativeY),
						SU_TO_FLOAT(currentSegmentRelativeX * laneBufferX - slideLineThickness), SU_TO_FLOAT(laneBufferY * currentSegmentRelativeY),
						SU_TO_FLOAT(currentSegmentRelativeX * laneBufferX + slideLineThickness), SU_TO_FLOAT(laneBufferY * currentSegmentRelativeY),
						slideLineColor, 16
					);
				}
				lastSegmentPosition = segmentPosition;
				lastSegmentRelativeX = currentSegmentRelativeX;
				lastSegmentRelativeY = currentSegmentRelativeY;
			}
			lastStep = slideElement;
		}
	}

	// Tap
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	for (int si = note->ExtraData.size() - 1; si >= 0; --si) {
		const auto& slideElement = note->ExtraData[si];
		if (slideElement->Type.test(size_t(SusNoteType::Control))) continue;
		if (slideElement->Type.test(size_t(SusNoteType::Injection))) continue;
		if (slideElement->Type.test(size_t(SusNoteType::Invisible))) continue;
		if (slideElement->OnTheFlyData[size_t(NoteAttribute::Finished)]/* && ノーツがAttack以上の判定*/) continue;

		const auto currentStepRelativeY = 1.0 - slideElement->ModifiedPosition / seenDuration;
		if (currentStepRelativeY >= 0 && currentStepRelativeY < cullingLimit) {
			const int length = SU_TO_INT32(slideElement->Length);
			if (slideElement->Type.test(size_t(SusNoteType::Start))) {
				DrawTap(slideElement->StartLane, length, currentStepRelativeY, imageSlide->GetHandle());
			}
			else {
				DrawTap(slideElement->StartLane, length, currentStepRelativeY, imageSlideStep->GetHandle());
			}
		}
	}
	if (!(note->OnTheFlyData[size_t(NoteAttribute::Finished)]/* && ノーツがAttack以上の判定*/)) {
		const int length = SU_TO_INT32(note->Length);
		DrawTap(note->StartLane, length, 1.0 - note->ModifiedPosition / seenDuration, imageSlide->GetHandle());
	}
}

void ScenePlayer::DrawAirActionStart(const AirDrawQuery & query) const
{
	const auto lastStep = query.Note;
	const auto lastStepRelativeY = query.Z;

	const auto aasz = glm::mix(SU_LANE_Z_MAX, SU_LANE_Z_MIN, lastStepRelativeY);
	const auto cry = (double(lastStep->StartLane) + lastStep->Length / 2.0) / 16.0;
	const auto center = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, cry);
	VERTEX3D vertices[] = {
		{
			VGet(center - 10, SU_LANE_Y_GROUND, aasz),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			0.9375f, 1.0f, 1.0f, 0.0f
		},
		{
			VGet(center - 10, SU_TO_FLOAT(SU_LANE_Y_AIR * lastStep->ExtraAttribute->HeightScale), aasz),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			0.9375f, 0.0f, 0.0f, 0.0f
		},
		{
			VGet(center + 10, SU_TO_FLOAT(SU_LANE_Y_AIR * lastStep->ExtraAttribute->HeightScale), aasz),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			1.0000f, 0.0f, 0.0f, 0.0f
		},
		{
			VGet(center + 10, SU_LANE_Y_GROUND, aasz),
			VGet(0, 0, -1),
			GetColorU8(255, 255, 255, 255),
			GetColorU8(0, 0, 0, 0),
			1.0000f, 1.0f, 0.0f, 0.0f
		},
	};
	DrawPolygonIndexed3D(vertices, 4, rectVertexIndices, 2, imageAirAction->GetHandle(), TRUE);
}

void ScenePlayer::DrawAirActionStepBox(const AirDrawQuery & query) const
{
	const auto slideElement = query.Note;
	const auto currentStepRelativeY = query.Z;

	SetUseZBuffer3D(TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	if (!slideElement->Type.test(size_t(SusNoteType::Invisible))) {
		const auto atLeft = (slideElement->StartLane) / 16.0;
		const auto atRight = (slideElement->StartLane + slideElement->Length) / 16.0;
		const auto left = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, atLeft) + 5;
		const auto right = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, atRight) - 5;
		const auto z = glm::mix(SU_LANE_Z_MAX, SU_LANE_Z_MIN, currentStepRelativeY);
		const auto color = GetColorU8(255, 255, 255, 255);
		const auto yBase = SU_TO_FLOAT(SU_LANE_Y_AIR * slideElement->ExtraAttribute->HeightScale);
		VERTEX3D vertices[] = {
			{ VGet(left, SU_LANE_Y_GROUND, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.625f, 1.0f, 0.0f, 0.0f },
		{ VGet(left, yBase, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.625f, 0.0f, 0.0f, 0.0f },
		{ VGet(left, yBase, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.0f, 0.25f, 0.0f, 0.0f },
		{ VGet(left, yBase + 20, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.125f, 0.25f, 0.0f, 0.0f },
		{ VGet(left, yBase + 20, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.208f, 1.0f, 0.0f, 0.0f },
		{ VGet(left, yBase + 40, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.208f, 0.5f, 0.0f, 0.0f },
		{ VGet(left, yBase, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.0f, 0.0f, 0.0f, 0.0f },
		{ VGet(left, yBase + 20, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.125f, 0.0f, 0.0f, 0.0f },
		{ VGet(left, yBase + 20, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.0f, 1.0f, 0.0f, 0.0f },
		{ VGet(left, yBase + 40, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.0f, 0.5f, 0.0f, 0.0f },

		{ VGet(right, SU_LANE_Y_GROUND, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.875f, 1.0f, 0.0f, 0.0f },
		{ VGet(right, yBase, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.875f, 0.0f, 0.0f, 0.0f },
		{ VGet(right, yBase, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.625f, 0.25f, 0.0f, 0.0f },
		{ VGet(right, yBase + 20, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.5f, 0.25f, 0.0f, 0.0f },
		{ VGet(right, yBase + 20, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.625f, 1.0f, 0.0f, 0.0f },
		{ VGet(right, yBase + 40, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.625f, 0.5f, 0.0f, 0.0f },
		{ VGet(right, yBase, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.625f, 0.0f, 0.0f, 0.0f },
		{ VGet(right, yBase + 20, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.5f, 0.0f, 0.0f, 0.0f },
		{ VGet(right, yBase + 20, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.416f, 1.0f, 0.0f, 0.0f },
		{ VGet(right, yBase + 40, z + 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.416f, 0.5f, 0.0f, 0.0f },

		{ VGet(left, yBase, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.125f, 0.5f, 0.0f, 0.0f },
		{ VGet(right, yBase, z - 20), VGet(0, 0, -1), color, GetColorU8(0, 0, 0, 0), 0.5f, 0.5f, 0.0f, 0.0f },
		};
		uint16_t indices[] = {
			// 本当は上2ついらないけどindex計算が面倒なので放置
			//下のやつ
			0, 1, 11,
			0, 11, 10,
			//本体
			//上
			3, 7, 17,
			3, 17, 13,
			//左
			6, 7, 3,
			6, 3, 2,
			//右
			12, 13, 17,
			12, 17, 16,
			//手前
			20, 3, 13,
			20, 13, 21,

			//へばりついてるの
			//手前
			4, 5, 15,
			4, 15, 14,
			//後ろ
			8, 9, 19,
			8, 19, 18,
			//左
			8, 9, 5,
			8, 5, 4,
			//右
			14, 15, 19,
			14, 19, 18,
		};
		SetUseZBuffer3D(TRUE);
		DrawPolygonIndexed3D(vertices, 22, indices + 6, 16, imageAirAction->GetHandle(), TRUE);
	}
}

void ScenePlayer::DrawAirActionStep(const AirDrawQuery & query) const
{
	const auto slideElement = query.Note;
	const auto currentStepRelativeY = query.Z;

	SetUseZBuffer3D(TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	if (!slideElement->Type.test(size_t(SusNoteType::Invisible))) {
		const auto atLeft = (slideElement->StartLane) / 16.0;
		const auto atRight = (slideElement->StartLane + slideElement->Length) / 16.0;
		const auto left = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, atLeft) + 5;
		const auto right = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, atRight) - 5;
		const auto z = glm::mix(SU_LANE_Z_MAX, SU_LANE_Z_MIN, currentStepRelativeY);
		const auto yBase = SU_TO_FLOAT(SU_LANE_Y_AIR * slideElement->ExtraAttribute->HeightScale);
		VERTEX3D vertices[] = {
			VERTEX3D { VGet(left, SU_LANE_Y_GROUND, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.625f, 1.0f, 0.0f, 0.0f },
			VERTEX3D { VGet(left, yBase, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.625f, 0.0f, 0.0f, 0.0f },
			VERTEX3D { VGet(right, SU_LANE_Y_GROUND, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.875f, 1.0f, 0.0f, 0.0f },
			VERTEX3D { VGet(right, yBase, z), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.875f, 0.0f, 0.0f, 0.0f },
		};
		uint16_t indices[] = {
			0, 1, 3,
			0, 3, 2,
		};
		SetUseZBuffer3D(FALSE);
		DrawPolygonIndexed3D(vertices, 4, indices, 2, imageAirAction->GetHandle(), TRUE);
	}
}

void ScenePlayer::DrawAirActionCover(const AirDrawQuery & query)
{
	const auto slideElement = query.Note;
	const auto lastStep = query.PreviousNote;
	const auto& segmentPositions = curveData[slideElement];

	auto lastSegmentPosition = segmentPositions[0];
	const auto blockDuration = slideElement->StartTime - lastStep->StartTime;
	auto lastSegmentLength = double(lastStep->Length);
	auto lastTimeInBlock = get<0>(lastSegmentPosition) / blockDuration;
	auto lastSegmentRelativeY = 1.0 - lastStep->ModifiedPosition / seenDuration;
	for (const auto& segmentPosition : segmentPositions) {
		if (lastSegmentPosition == segmentPosition) continue;
		const auto currentTimeInBlock = get<0>(segmentPosition) / (slideElement->StartTime - lastStep->StartTime);
		const auto currentSegmentLength = glm::mix(double(lastStep->Length), double(slideElement->Length), currentTimeInBlock);
		const auto segmentExPosition = glm::mix(lastStep->ModifiedPosition, slideElement->ModifiedPosition, currentTimeInBlock);
		const auto currentSegmentRelativeY = 1.0 - segmentExPosition / seenDuration;

		if ((currentSegmentRelativeY >= 0 || lastSegmentRelativeY >= 0)
			&& (currentSegmentRelativeY < cullingLimit || lastSegmentRelativeY < cullingLimit)) {
			SetUseZBuffer3D(FALSE);
			SetUseBackCulling(FALSE);
			const auto back = glm::mix(SU_LANE_Z_MAX, SU_LANE_Z_MIN, currentSegmentRelativeY);
			const auto front = glm::mix(SU_LANE_Z_MAX, SU_LANE_Z_MIN, lastSegmentRelativeY);
			const auto backLeft = get<1>(segmentPosition) - currentSegmentLength / 32.0;
			const auto backRight = get<1>(segmentPosition) + currentSegmentLength / 32.0;
			const auto frontLeft = get<1>(lastSegmentPosition) - lastSegmentLength / 32.0;
			const auto frontRight = get<1>(lastSegmentPosition) + lastSegmentLength / 32.0;
			const auto pbl = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, backLeft);
			const auto pbr = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, backRight);
			const auto pfl = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, frontLeft);
			const auto pfr = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, frontRight);
			const auto pbz = SU_TO_FLOAT(glm::mix(lastStep->ExtraAttribute->HeightScale, slideElement->ExtraAttribute->HeightScale, currentTimeInBlock));
			const auto pfz = SU_TO_FLOAT(glm::mix(lastStep->ExtraAttribute->HeightScale, slideElement->ExtraAttribute->HeightScale, lastTimeInBlock));
			VERTEX3D vertices[] = {
				VERTEX3D { VGet(pfl, SU_LANE_Y_AIR * pfz, front), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.875f, 1.0f, 1.0f, 0.0f },
				VERTEX3D { VGet(pbl, SU_LANE_Y_AIR * pbz, back), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.875f, 0.0f, 0.0f, 0.0f },
				VERTEX3D { VGet(pbr, SU_LANE_Y_AIR * pbz, back), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.9375f, 0.0f, 0.0f, 0.0f },
				VERTEX3D { VGet(pfr, SU_LANE_Y_AIR * pfz, front), VGet(0, 0, -1), GetColorU8(255, 255, 255, 255), GetColorU8(0, 0, 0, 0), 0.9375f, 1.0f, 0.0f, 0.0f },
			};
			DrawPolygonIndexed3D(vertices, 4, rectVertexIndices, 2, imageAirAction->GetHandle(), TRUE);

			vertices[0].pos.x = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, get<1>(lastSegmentPosition)) - 10;
			vertices[1].pos.x = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, get<1>(segmentPosition)) - 10;
			vertices[2].pos.x = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, get<1>(segmentPosition)) + 10;
			vertices[3].pos.x = glm::mix(SU_LANE_X_MIN, SU_LANE_X_MAX, get<1>(lastSegmentPosition)) + 10;
			vertices[0].u = 0.9375f; vertices[0].v = 1.0f;
			vertices[1].u = 0.9375f; vertices[1].v = 0.0f;
			vertices[2].u = 1.0000f; vertices[2].v = 0.0f;
			vertices[3].u = 1.0000f; vertices[3].v = 1.0f;
			DrawPolygonIndexed3D(vertices, 4, rectVertexIndices, 2, imageAirAction->GetHandle(), TRUE);
		}

		lastSegmentPosition = segmentPosition;
		lastSegmentLength = currentSegmentLength;
		lastSegmentRelativeY = currentSegmentRelativeY;
		lastTimeInBlock = currentTimeInBlock;
	}
}

void ScenePlayer::DrawTap(const float lane, const int length, const double relpos, const int handle) const
{
	for (auto i = 0; i < length * 2; i++) {
		const auto type = i ? (i == length * 2 - 1 ? 2 : 1) : 0;
		DrawRectRotaGraph3F(
			(lane * 2 + i) * widthPerLane / 2, SU_TO_FLOAT(laneBufferY * relpos),
			SU_TO_INT32(noteImageBlockX * type), (0),
			SU_TO_INT32(noteImageBlockX), SU_TO_INT32(noteImageBlockY),
			0, noteImageBlockY / 2,
			actualNoteScaleX, actualNoteScaleY, 0,
			handle, TRUE, FALSE);
	}
}

void ScenePlayer::DrawMeasureLine(const shared_ptr<SusDrawableNoteData> & note) const
{
	const auto relpos = SU_TO_FLOAT(1.0 - note->ModifiedPosition / seenDuration);
	DrawLineAA(0, relpos * laneBufferY, laneBufferX, relpos * laneBufferY, GetColor(255, 255, 255), 6);
}

void ScenePlayer::Prepare3DDrawCall() const
{
	SetUseLighting(FALSE);
	SetCameraPositionAndTarget_UpVecY(VGet(0, SU_TO_FLOAT(cameraY), SU_TO_FLOAT(cameraZ)), VGet(0, SU_LANE_Y_GROUND, SU_TO_FLOAT(cameraTargetZ)));
}
