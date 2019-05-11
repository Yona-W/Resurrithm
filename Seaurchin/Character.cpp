#include "Character.h"
#include "CharacterInstance.h"
#include "ExecutionManager.h"
#include "Setting.h"
#include "Skill.h"

using namespace std;

void CharacterImageMetric::RegisterType(asIScriptEngine* engine)
{
	engine->RegisterObjectType(SU_IF_CHARACTER_METRIC, sizeof(CharacterImageMetric), asOBJ_VALUE | asOBJ_POD);
	engine->RegisterObjectProperty(SU_IF_CHARACTER_METRIC, "double WholeScale", asOFFSET(CharacterImageMetric, WholeScale));
	engine->RegisterObjectMethod(SU_IF_CHARACTER_METRIC, "int get_FaceOrigin(uint)", asMETHOD(CharacterImageMetric, GetFaceOrigin), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_METRIC, "int get_SmalLRange(uint)", asMETHOD(CharacterImageMetric, GetSmallRange), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_METRIC, "int get_FaceRange(uint)", asMETHOD(CharacterImageMetric, GetFaceRange), asCALL_THISCALL);
}


void CharacterParameter::RegisterType(asIScriptEngine* engine)
{
	engine->RegisterObjectType(SU_IF_CHARACTER_PARAM, 0, asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterObjectProperty(SU_IF_CHARACTER_PARAM, "string Name", asOFFSET(CharacterParameter, Name));
	engine->RegisterObjectProperty(SU_IF_CHARACTER_PARAM, "string ImagePath", asOFFSET(CharacterParameter, ImagePath));
	engine->RegisterObjectProperty(SU_IF_CHARACTER_PARAM, SU_IF_CHARACTER_METRIC " Metric", asOFFSET(CharacterParameter, Metric));
}


CharacterImageSet::CharacterImageSet(const shared_ptr<CharacterParameter> param)
{
	parameter = param;
	LoadAllImage();
}

CharacterImageSet::~CharacterImageSet()
{
	imageFull->Release();
	imageSmall->Release();
	imageFace->Release();
}

void CharacterImageSet::ApplyFullImage(SSprite * sprite) const
{
	const auto cx = parameter->Metric.FaceOrigin[0];
	const auto cy = parameter->Metric.FaceOrigin[1];
	const auto sc = parameter->Metric.WholeScale;
	ostringstream ss;
	ss << "origX:" << cx << ", origY:" << cy << ", scaleX:" << sc << ", scaleY:" << sc;

	imageFull->AddRef();
	sprite->SetImage(imageFull);
	sprite->Apply(ss.str());
	sprite->Release();
}

void CharacterImageSet::ApplySmallImage(SSprite * sprite) const
{
	imageSmall->AddRef();
	sprite->SetImage(imageSmall);
	sprite->Release();
}

void CharacterImageSet::ApplyFaceImage(SSprite * sprite) const
{
	imageFace->AddRef();
	sprite->SetImage(imageFace);
	sprite->Release();
}

CharacterImageSet* CharacterImageSet::CreateImageSet(const shared_ptr<CharacterParameter> param)
{
	auto result = new CharacterImageSet(param);
	result->AddRef();
	return result;
}

void CharacterImageSet::LoadAllImage()
{
	const auto hBase = LoadGraph(parameter->ImagePath.c_str());
	const auto hSmall = MakeScreen(SU_CHAR_SMALL_WIDTH, SU_CHAR_SMALL_WIDTH, 1);
	const auto hFace = MakeScreen(SU_CHAR_FACE_SIZE, SU_CHAR_FACE_SIZE, 1);
	BEGIN_DRAW_TRANSACTION(hSmall);
	DrawRectExtendGraph(
		0, 0, SU_CHAR_SMALL_WIDTH, SU_CHAR_SMALL_HEIGHT,
		parameter->Metric.SmallRange[0], parameter->Metric.SmallRange[1],
		parameter->Metric.SmallRange[2], parameter->Metric.SmallRange[3],
		hBase, TRUE);
	BEGIN_DRAW_TRANSACTION(hFace);
	DrawRectExtendGraph(
		0, 0, SU_CHAR_FACE_SIZE, SU_CHAR_FACE_SIZE,
		parameter->Metric.FaceRange[0], parameter->Metric.FaceRange[1],
		parameter->Metric.FaceRange[2], parameter->Metric.FaceRange[3],
		hBase, TRUE);
	FINISH_DRAW_TRANSACTION;
	imageFull = new SImage(hBase);
	imageFull->AddRef();
	imageSmall = new SImage(hSmall);
	imageSmall->AddRef();
	imageFace = new SImage(hFace);
	imageFace->AddRef();
}

void CharacterImageSet::RegisterType(asIScriptEngine * engine)
{
	engine->RegisterObjectType(SU_IF_CHARACTER_IMAGES, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_CHARACTER_IMAGES, asBEHAVE_ADDREF, "void f()", asMETHOD(CharacterImageSet, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_CHARACTER_IMAGES, asBEHAVE_RELEASE, "void f()", asMETHOD(CharacterImageSet, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_IMAGES, "void ApplyFullImage(" SU_IF_SPRITE "@)", asMETHOD(CharacterImageSet, ApplyFullImage), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_IMAGES, "void ApplySmallImage(" SU_IF_SPRITE "@)", asMETHOD(CharacterImageSet, ApplySmallImage), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CHARACTER_IMAGES, "void ApplyFaceImage(" SU_IF_SPRITE "@)", asMETHOD(CharacterImageSet, ApplyFaceImage), asCALL_THISCALL);
}
