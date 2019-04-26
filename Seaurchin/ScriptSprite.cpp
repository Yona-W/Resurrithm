#include "ScriptSprite.h"
#include "ScriptSpriteMover.h"
#include "ExecutionManager.h"
#include "Misc.h"

using namespace std;
using namespace crc32_constexpr;

// パーサー
namespace {
#define SKIP_SP(p) while(*p != '\0' && !isgraph(*p)) ++p
#define CHECK_END(p) do { if (*p == '\0') return false; } while(0)

	using namespace std;

	inline bool Double(const char* expression, const char** seeked, double& retVal)
	{
		const char* p = expression;

		SKIP_SP(p);
		CHECK_END(p);

		bool isPositive = false, isNegative = false;
		if (*p == '+') {
			isPositive = true;
			++p;
		}
		else if (*p == '-') {
			isNegative = true;
			++p;
		}

		if (isdigit(*p) || *p == '.') {
			double val = 0.0;
			while (isdigit(*p)) {
				val = val * 10.0 + (*p - '0');
				++p;
			}
			if (*p == '.') {
				++p;

				double dig = 0.1;
				while (isdigit(*p)) {
					val += (*p - '0') * dig;
					dig *= 0.1;
					++p;
				}
			}

			if (isNegative) val *= -1.0;

			retVal = val;
			*seeked = p;
			return true;
		}

		return false;
	}
	inline bool String(const char* expression, const char** seeked, string & retVal)
	{
		const char* p = expression;

		SKIP_SP(p);
		CHECK_END(p);

		if (isalpha(*p) || *p == '_' || *p == '@') {
			const char* head = p;
			do ++p; while (isalnum(*p) || *p == '_' || *p == '@');
			const char* tail = p;
			const size_t len = tail - head;

			*seeked = p;
			retVal = string(head, len);
			return true;
		}

		return false;
	}
	inline bool FieldID(const char* expression, const char** seeked, SSprite::FieldID & retVal)
	{
		const char* p = expression;

		SKIP_SP(p);
		CHECK_END(p);

		if (isalpha(*p)) {
			const char* head = p;
			do ++p; while (isalnum(*p));
			const char* tail = p;
			const size_t len = tail - head;

			{
				char* buf = (char*)malloc(sizeof(char) * (len + 1));
				if (buf == NULL) return false;

				memcpy(buf, head, len);
				buf[len] = '\0';

				*seeked = p;
				retVal = SSprite::GetFieldId(buf);
				free(buf);
			}
			return true;
		}

		return false;
	}

	inline bool ApplyPair(const char* expression, const char** seeked, SSprite::FieldID & key, double& dVal)
	{
		const char* p = expression;

		if (!FieldID(p, &p, key)) return false;

		SKIP_SP(p);
		CHECK_END(p);

		if (*p != ':') return false;
		++p;
		CHECK_END(p);

		{
			double dv;
			if (Double(p, &p, dv)) {
				*seeked = p;
				dVal = dv;
				return true;
			}
		}

		return false;
	}
	inline bool Apply(const char* expression, SSprite * pSprite)
	{
		const char* p = expression;

		SKIP_SP(p);

		while (*p != '\0') {
			SSprite::FieldID key;
			double dVal;

			if (!ApplyPair(p, &p, key, dVal)) return false;

			if (!pSprite->Apply(key, dVal)) {
				// NOTE: Applyがログ出すからそれでいいかな
			}

			SKIP_SP(p);

			if (*p == '\0') break;
			if (*p != ',') return false;

			++p;
			CHECK_END(p);
		}

		return true;
	}

	inline bool AddMoveObjectPair(const char* expression, const char** seeked, string & key, double& dVal, string & sVal, bool& isStr)
	{
		const char* p = expression;

		if (!String(p, &p, key)) return false;

		SKIP_SP(p);
		CHECK_END(p);

		if (*p != ':') return false;
		++p;
		CHECK_END(p);

		{
			double dv;
			if (Double(p, &p, dv)) {
				*seeked = p;
				isStr = false;
				dVal = dv;
				return true;
			}
		}

		{
			string sv;
			if (String(p, &p, sv)) {
				*seeked = p;
				isStr = true;
				sVal = sv;
				return true;
			}
		}

		return false;
	}
	inline bool AddMoveObject(const char* expression, const char** seeked, MoverObject * pMoverObject)
	{
		const char* p = expression;

		SKIP_SP(p);

		while (*p != '\0') {
			string key;
			double dVal;
			string sVal;
			bool isStr;

			if (!AddMoveObjectPair(p, &p, key, dVal, sVal, isStr)) return false;

			if (isStr) {
				if (!pMoverObject->Apply(key, sVal)) {
					// NOTE: Applyがログ出すからそれでいいかな
				}
			}
			else {
				if (!pMoverObject->Apply(key, dVal)) {
					// NOTE: Applyがログ出すからそれでいいかな
				}
			}

			SKIP_SP(p);

			if (*p == '\0') break;
			if (*p != ',') break;

			++p;
			CHECK_END(p);
		}

		*seeked = p;
		return true;
	}
	inline bool AddMovePair(const char* expression, const char** seeked, SSpriteMover * pMover)
	{
		const char* p = expression;
		vector<SSprite::FieldID> fields; // NOTE: 8個も取っておけばそうそうリサイズされないでしょという気持ち

		SKIP_SP(p);
		CHECK_END(p);

		do {
			SSprite::FieldID id;
			if (!FieldID(p, &p, id)) return false;
			fields.emplace_back(id);

			SKIP_SP(p);
			CHECK_END(p);

			if (*p == ':') {
				++p;
				break;
			}
			if (*p != ',') return false;

			++p;
			CHECK_END(p);
		} while (*p != '\0');

		SKIP_SP(p);
		CHECK_END(p);

		if (*p != '{') return false;
		++p;

		SKIP_SP(p);
		CHECK_END(p);

		MoverObject * pMoverObject = new MoverObject();
		bool retVal = false;
		while (*p != '\0') {
			if (!AddMoveObject(p, &p, pMoverObject)) break;

			SKIP_SP(p);
			if (*p == '\0') break;

			if (*p == '}') {
				++p;
				retVal = true;
				break;
			}
			if (*p != ',') break;

			++p;
			SKIP_SP(p);
			if (*p == '}') break;
		}

		if (!retVal) {
			pMoverObject->Release();
			return false;
		}

		for (auto it = fields.begin(); it != fields.end(); ++it) {
			// TODO: 同じパラメータを持つ異なるフィールドのMoverObjectを複数登録するのではなく、MoverObjectが複数のフィールドを一括制御できるようにする
			MoverObject* pTmpMoverObj = pMoverObject->Clone();
			pTmpMoverObj->RegisterTargetField(*it);
			pMover->AddMove(pTmpMoverObj);
		}
		pMoverObject->Release();

		*seeked = p;
		return true;
	}
	inline bool AddMove(const char* expression, SSpriteMover * pMover)
	{
		const char* p = expression;

		while (*p != '\0') {
			if (!AddMovePair(p, &p, pMover)) return false;

			if (*p == '\0') break;
			SKIP_SP(p);

			if (*p == '\0') break;
			if (*p != ',') return false;

			++p;
			CHECK_END(p);
		}

		return true;
	}

#undef SKIP_SP
#undef CHECK_END
}

// 一般

void RegisterScriptSprite(ExecutionManager * exm)
{
	const auto engine = exm->GetScriptInterfaceUnsafe()->GetEngine();

	MoverObject::RegisterType(engine);

	SSprite::RegisterType(engine);
	SShape::RegisterType(engine);
	STextSprite::RegisterType(engine);
	SSynthSprite::RegisterType(engine);
	SClippingSprite::RegisterType(engine);
	SAnimeSprite::RegisterType(engine);
	SContainer::RegisterType(engine);
}

//SSprite ------------------

bool SSprite::GetField(const SSprite * pSprite, FieldID id, double& value)
{
	if (!pSprite) return false;

	const SShape* pShape = dynamic_cast<const SShape*>(pSprite);
	const SClippingSprite* pClip = dynamic_cast<const SClippingSprite*>(pSprite);
	const SAnimeSprite* pAnim = dynamic_cast<const SAnimeSprite*>(pSprite);

	switch (id)
	{
	case FieldID::X: value = SU_TO_DOUBLE(pSprite->Transform.X); return true;
	case FieldID::Y: value = SU_TO_DOUBLE(pSprite->Transform.Y); return true;
	case FieldID::Z: value = SU_TO_DOUBLE(pSprite->ZIndex); return true;
	case FieldID::OriginX: value = SU_TO_DOUBLE(pSprite->Transform.OriginX); return true;
	case FieldID::OriginY: value = SU_TO_DOUBLE(pSprite->Transform.OriginY); return true;
	case FieldID::Angle: value = SU_TO_DOUBLE(pSprite->Transform.Angle); return true;
	case FieldID::Scale: value = SU_TO_DOUBLE(pSprite->Transform.ScaleX + pSprite->Transform.ScaleY) / 2.0; return true;
	case FieldID::ScaleX: value = SU_TO_DOUBLE(pSprite->Transform.ScaleX); return true;
	case FieldID::ScaleY: value = SU_TO_DOUBLE(pSprite->Transform.ScaleY); return true;
	case FieldID::Alpha: value = SU_TO_DOUBLE(pSprite->Color.A / 255.0); return true;
	case FieldID::R: value = SU_TO_DOUBLE(pSprite->Color.R); return true;
	case FieldID::G: value = SU_TO_DOUBLE(pSprite->Color.G); return true;
	case FieldID::B: value = SU_TO_DOUBLE(pSprite->Color.B); return true;

	case FieldID::Death: value = pSprite->IsDead ? 1.0 : 0.0; return true;

	case FieldID::Width: if (!pShape) return false; value = SU_TO_DOUBLE(pShape->Width); return true;
	case FieldID::Height: if (!pShape) return false; value = SU_TO_DOUBLE(pShape->Height); return true;

	case FieldID::U1: if (!pClip) return false; value = pClip->GetU1(); return true;
	case FieldID::V1: if (!pClip) return false; value = pClip->GetV1(); return true;
	case FieldID::U2: if (!pClip) return false; value = pClip->GetU2(); return true;
	case FieldID::V2: if (!pClip) return false; value = pClip->GetV2(); return true;

	case FieldID::LoopCount: if (!pAnim) return false; value = pAnim->GetLoopCount(); return true;
	case FieldID::Speed: if (!pAnim) return false; value = pAnim->GetSpeed(); return true;

	default: return false;
	}
}

bool SSprite::SetField(SSprite * pSprite, FieldID id, double value)
{
	if (!pSprite) {
		spdlog::get("main")->critical(u8"なんで");
		return false;
	}

	SShape* pShape = dynamic_cast<SShape*>(pSprite);
	SClippingSprite* pClip = dynamic_cast<SClippingSprite*>(pSprite);
	SAnimeSprite* pAnim = dynamic_cast<SAnimeSprite*>(pSprite);

	switch (id) {
	case FieldID::X: return pSprite->SetPosX(value);
	case FieldID::Y: return pSprite->SetPosY(value);
	case FieldID::Z: return pSprite->SetZIndex(value);
	case FieldID::OriginX: return pSprite->SetOriginX(value);
	case FieldID::OriginY: return pSprite->SetOriginY(value);
	case FieldID::Angle: return pSprite->SetAngle(value);
	case FieldID::Scale: return pSprite->SetScale(value);
	case FieldID::ScaleX: return pSprite->SetScaleX(value);
	case FieldID::ScaleY: return pSprite->SetScaleY(value);
	case FieldID::Alpha: return pSprite->SetAlpha(value);
	case FieldID::R: return pSprite->SetColorR(value);
	case FieldID::G: return pSprite->SetColorG(value);
	case FieldID::B: return pSprite->SetColorB(value);

	case FieldID::Death: pSprite->Dismiss(); return true;

	case FieldID::Width: if (!pShape) return false; return pShape->SetWidth(value);
	case FieldID::Height: if (!pShape) return false; return pShape->SetHeight(value);

	case FieldID::U1: if (!pClip) return false; return pClip->SetU1(value);
	case FieldID::V1: if (!pClip) return false; return pClip->SetV1(value);
	case FieldID::U2: if (!pClip) return false; return pClip->SetU2(value);
	case FieldID::V2: if (!pClip) return false; return pClip->SetV2(value);

	case FieldID::LoopCount: if (!pAnim) return false; return pAnim->SetLoopCount(value);
	case FieldID::Speed: if (!pAnim) return false; return pAnim->SetSpeed(value);

	default: return false;
	}
}

void SSprite::CopyParameterFrom(SSprite * original)
{
	Transform = original->Transform;
	ZIndex = original->ZIndex;
	Color = original->Color;
	IsDead = original->IsDead;
	HasAlpha = original->HasAlpha;
}

void SSprite::SetImage(SImage * img)
{
	if (Image) Image->Release();
	Image = img;
}

const SImage* SSprite::GetImage() const
{
	if (Image) Image->AddRef();
	return Image;
}

SSprite::SSprite()
	: reference(0)
	, pMover(new SSpriteMover(this)) // NOTE: this渡すのはちょっと怖いけど
	, ZIndex(0)
	, Color(Colors::white)
	, IsDead(false)
	, HasAlpha(true)
	, Image(nullptr)
{
}

SSprite::~SSprite()
{
	if (Image) Image->Release();
	Image = nullptr;
	delete pMover;
}

void SSprite::AddRef()
{
	++reference;
}

void SSprite::Release()
{
	if (--reference == 0) delete this;
}

bool SSprite::Apply(FieldID id, const double value)
{
	return SSprite::SetField(this, id, value);
}

void SSprite::Apply(const string & dict)
{
	::Apply(dict.c_str(), this);
	return;
}

void SSprite::Apply(const CScriptDictionary * dict)
{
	auto i = dict->begin();
	while (i != dict->end()) {
		const auto key = i.GetKey();
		const auto id = SSprite::GetFieldId(key);
		double dv = 0;
		if (i.GetValue(dv)) if (!Apply(id, dv)) spdlog::get("main")->warn(u8"プロパティに \"{0}\" は設定できません。", key);
		++i;
	}

	dict->Release();
}

void SSprite::AddMove(const string & dict)
{
	pMover->AddMove(dict);
}

void SSprite::AddMove(const string & key, const CScriptDictionary * dict)
{
	dict->AddRef();
	pMover->AddMove(key, dict);

	dict->Release();
}

void SSprite::AddMove(const string & key, MoverObject * pMoverObj)
{
	if (!pMoverObj) return;

	MoverObject* pClone = pMoverObj->Clone();

	SSprite::FieldID id = SSprite::GetFieldId(key);
	if (!pClone->RegisterTargetField(id)) {
		pClone->Release();
		pMoverObj->Release();
		return;
	}

	pMover->AddMove(pClone);

	pMoverObj->Release();
}

void SSprite::AbortMove(const bool terminate)
{
	pMover->Abort(terminate);
}

void SSprite::Tick(const double delta)
{
	pMover->Tick(delta);
}

void SSprite::Draw()
{
	DrawBy(Transform, Color);
}

void SSprite::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	DrawBy(tf, cl);
}

void SSprite::DrawBy(const Transform2D & tf, const ColorTint & ct)
{
	if (!Image) return;
	SetDrawBright(ct.R, ct.G, ct.B);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ct.A);
	DrawRotaGraph3F(
		tf.X, tf.Y,
		tf.OriginX, tf.OriginY,
		tf.ScaleX, tf.ScaleY,
		tf.Angle, Image->GetHandle(),
		HasAlpha ? TRUE : FALSE, FALSE);
}

SSprite* SSprite::Clone()
{
	//コピコンで良くないかこれ
	auto clone = new SSprite();
	clone->AddRef();

	clone->CopyParameterFrom(this);

	{
		if (Image) Image->AddRef();
		clone->SetImage(Image);
	}

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

SSprite* SSprite::Factory()
{
	auto result = new SSprite();
	result->AddRef();

	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

SSprite* SSprite::Factory(SImage * img)
{
	auto result = new SSprite();
	result->AddRef();

	{
		if (img) img->AddRef();
		result->SetImage(img);
	}

	if (img) img->Release();
	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

ColorTint GetColor(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	return ColorTint{ a, r, g, b };
}

void SSprite::RegisterType(asIScriptEngine * engine)
{
	//Transform2D
	engine->RegisterObjectType(SU_IF_TF2D, sizeof(Transform2D), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
	engine->RegisterObjectBehaviour(SU_IF_TF2D, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(AngelScriptValueConstruct<Transform2D>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour(SU_IF_TF2D, asBEHAVE_DESTRUCT, "void f()", asFUNCTION(AngelScriptValueDestruct<Transform2D>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectProperty(SU_IF_TF2D, "float X", asOFFSET(Transform2D, X));
	engine->RegisterObjectProperty(SU_IF_TF2D, "float Y", asOFFSET(Transform2D, Y));
	engine->RegisterObjectProperty(SU_IF_TF2D, "float Angle", asOFFSET(Transform2D, Angle));
	engine->RegisterObjectProperty(SU_IF_TF2D, "float OriginX", asOFFSET(Transform2D, OriginX));
	engine->RegisterObjectProperty(SU_IF_TF2D, "float OriginY", asOFFSET(Transform2D, OriginY));
	engine->RegisterObjectProperty(SU_IF_TF2D, "float ScaleX", asOFFSET(Transform2D, ScaleX));
	engine->RegisterObjectProperty(SU_IF_TF2D, "float ScaleY", asOFFSET(Transform2D, ScaleY));

	//Color
	engine->RegisterObjectType(SU_IF_COLOR, sizeof(ColorTint), asOBJ_VALUE | asOBJ_APP_CLASS_CD);
	engine->RegisterObjectBehaviour(SU_IF_COLOR, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(AngelScriptValueConstruct<ColorTint>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour(SU_IF_COLOR, asBEHAVE_DESTRUCT, "void f()", asFUNCTION(AngelScriptValueConstruct<ColorTint>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 A", asOFFSET(ColorTint, A));
	engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 R", asOFFSET(ColorTint, R));
	engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 G", asOFFSET(ColorTint, G));
	engine->RegisterObjectProperty(SU_IF_COLOR, "uint8 B", asOFFSET(ColorTint, B));

	//ShapeType
	engine->RegisterEnum(SU_IF_SHAPETYPE);
	engine->RegisterEnumValue(SU_IF_SHAPETYPE, "Pixel", int(SShapeType::Pixel));
	engine->RegisterEnumValue(SU_IF_SHAPETYPE, "Box", int(SShapeType::Box));
	engine->RegisterEnumValue(SU_IF_SHAPETYPE, "BoxFill", int(SShapeType::BoxFill));
	engine->RegisterEnumValue(SU_IF_SHAPETYPE, "Oval", int(SShapeType::Oval));
	engine->RegisterEnumValue(SU_IF_SHAPETYPE, "OvalFill", int(SShapeType::OvalFill));

	//ShapeType
	engine->RegisterEnum(SU_IF_TEXTALIGN);
	engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Top", int(STextAlign::Top));
	engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Bottom", int(STextAlign::Bottom));
	engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Left", int(STextAlign::Left));
	engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Right", int(STextAlign::Right));
	engine->RegisterEnumValue(SU_IF_TEXTALIGN, "Center", int(STextAlign::Center));

	RegisterSpriteBasic<SSprite>(engine, SU_IF_SPRITE);
	engine->RegisterObjectBehaviour(SU_IF_SPRITE, asBEHAVE_FACTORY, SU_IF_SPRITE "@ f()", asFUNCTIONPR(SSprite::Factory, (), SSprite*), asCALL_CDECL);
	engine->RegisterObjectBehaviour(SU_IF_SPRITE, asBEHAVE_FACTORY, SU_IF_SPRITE "@ f(" SU_IF_IMAGE "@)", asFUNCTIONPR(SSprite::Factory, (SImage*), SSprite*), asCALL_CDECL);
}

// Shape -----------------

SShape::SShape()
	: Type(SShapeType::BoxFill)
	, Width(32)
	, Height(32)
{
}

void SShape::DrawBy(const Transform2D & tf, const ColorTint & ct)
{
	SetDrawBright(ct.R, ct.G, ct.B);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ct.A);
	switch (Type) {
	case SShapeType::Pixel:
		DrawPixel(SU_TO_INT32(tf.X), SU_TO_INT32(tf.Y), GetColor(255, 255, 255));
		break;
	case SShapeType::Box: {
		const glm::vec2 points[] = {
			glm::rotate(glm::vec2(+Width * tf.ScaleX / 2.0, +Height * tf.ScaleY / 2.0), float(tf.Angle)),
			glm::rotate(glm::vec2(-Width * tf.ScaleX / 2.0, +Height * tf.ScaleY / 2.0), float(tf.Angle)),
			glm::rotate(glm::vec2(-Width * tf.ScaleX / 2.0, -Height * tf.ScaleY / 2.0), float(tf.Angle)),
			glm::rotate(glm::vec2(+Width * tf.ScaleX / 2.0, -Height * tf.ScaleY / 2.0), float(tf.Angle))
		};
		DrawQuadrangleAA(
			tf.X + points[0].x, tf.Y - points[0].y,
			tf.X + points[1].x, tf.Y - points[1].y,
			tf.X + points[2].x, tf.Y - points[2].y,
			tf.X + points[3].x, tf.Y - points[3].y,
			GetColor(255, 255, 255), FALSE
		);
		break;
	}
	case SShapeType::BoxFill: {
		const glm::vec2 points[] = {
			glm::rotate(glm::vec2(+Width * tf.ScaleX / 2.0, +Height * tf.ScaleY / 2.0), float(tf.Angle)),
			glm::rotate(glm::vec2(-Width * tf.ScaleX / 2.0, +Height * tf.ScaleY / 2.0), float(tf.Angle)),
			glm::rotate(glm::vec2(-Width * tf.ScaleX / 2.0, -Height * tf.ScaleY / 2.0), float(tf.Angle)),
			glm::rotate(glm::vec2(+Width * tf.ScaleX / 2.0, -Height * tf.ScaleY / 2.0), float(tf.Angle))
		};
		DrawQuadrangleAA(
			tf.X + points[0].x, tf.Y - points[0].y,
			tf.X + points[1].x, tf.Y - points[1].y,
			tf.X + points[2].x, tf.Y - points[2].y,
			tf.X + points[3].x, tf.Y - points[3].y,
			GetColor(255, 255, 255), TRUE
		);
		break;
	}
	case SShapeType::Oval: {
		auto prev = glm::rotate(
			glm::vec2(Width * tf.ScaleX / 2.0 * glm::cos(0), Height * tf.ScaleY / 2.0 * glm::sin(0)),
			float(tf.Angle)
		);
		for (auto i = 1; i <= 256; ++i) {
			const auto angle = 2.0 * glm::pi<double>() / 256.0 * i;
			const auto next = glm::rotate(
				glm::vec2(Width * tf.ScaleX / 2.0 * glm::cos(angle), Height * tf.ScaleY / 2.0 * glm::sin(angle)),
				float(tf.Angle)
			);
			DrawLineAA(
				float(tf.X + prev.x),
				float(tf.Y - prev.y),
				float(tf.X + next.x),
				float(tf.Y - next.y),
				GetColor(255, 255, 255)
			);
			prev = next;
		}
		break;
	}
	case SShapeType::OvalFill: {
		auto prev = glm::rotate(
			glm::vec2(Width * tf.ScaleX / 2.0 * glm::cos(0), Height * tf.ScaleY / 2.0 * glm::sin(0)),
			float(tf.Angle)
		);
		for (auto i = 1; i <= 256; ++i) {
			const auto angle = 2.0 * glm::pi<double>() / 256.0 * i;
			const auto next = glm::rotate(
				glm::vec2(Width * tf.ScaleX / 2.0 * glm::cos(angle), Height * tf.ScaleY / 2.0 * glm::sin(angle)),
				float(tf.Angle)
			);
			DrawTriangleAA(
				float(tf.X),
				float(tf.Y),
				float(tf.X + prev.x),
				float(tf.Y - prev.y),
				float(tf.X + next.x),
				float(tf.Y - next.y),
				GetColor(255, 255, 255),
				TRUE
			);
			prev = next;
		}
		break;
	}
	}
}

void SShape::Draw()
{
	DrawBy(Transform, Color);
}

void SShape::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	DrawBy(tf, cl);
}

SShape* SShape::Clone()
{
	//やっぱりコピコンで良くないかこれ
	auto clone = new SShape();
	clone->AddRef();

	clone->CopyParameterFrom(this);

	clone->Width = Width;
	clone->Height = Height;

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

SShape* SShape::Factory()
{
	auto result = new SShape();
	result->AddRef();

	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

void SShape::RegisterType(asIScriptEngine * engine)
{
	RegisterSpriteBasic<SShape>(engine, SU_IF_SHAPE);
	engine->RegisterObjectBehaviour(SU_IF_SHAPE, asBEHAVE_FACTORY, SU_IF_SHAPE "@ f()", asFUNCTIONPR(SShape::Factory, (), SShape*), asCALL_CDECL);
	engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_SHAPE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SShape>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_SHAPE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SShape, SSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectProperty(SU_IF_SHAPE, "double Width", asOFFSET(SShape, Width));
	engine->RegisterObjectProperty(SU_IF_SHAPE, "double Height", asOFFSET(SShape, Height));
	engine->RegisterObjectProperty(SU_IF_SHAPE, SU_IF_SHAPETYPE " Type", asOFFSET(SShape, Type));
}

// TextSprite -----------

void STextSprite::Refresh()
{
	delete target;
	delete scrollBuffer;
	if (!Font) {
		size = std::make_tuple<double, double, int>(0.0, 0.0, 0);
		return;
	}

	size = isRich ? Font->RenderRich(nullptr, Text, Color) : Font->RenderRaw(nullptr, Text);
	if (isScrolling) {
		scrollBuffer = new SRenderTarget(scrollWidth, int(get<1>(size)));
	}

	target = new SRenderTarget(int(get<0>(size)), int(get<1>(size)));
	if (isRich) {
		Font->RenderRich(target, Text, Color);
	}
	else {
		Font->RenderRaw(target, Text);
	}
}

void STextSprite::DrawNormal(const Transform2D & tf, const ColorTint & ct)
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ct.A);
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
	if (isRich) {
		SetDrawBright(255, 255, 255);
	}
	else {
		SetDrawBright(ct.R, ct.G, ct.B);
	}
	const auto tox = SU_TO_FLOAT(get<0>(size) / 2 * int(horizontalAlignment));
	const auto toy = SU_TO_FLOAT(get<1>(size) / 2 * int(verticalAlignment));
	DrawRotaGraph3F(
		tf.X, tf.Y,
		tf.OriginX + tox, tf.OriginY + toy,
		tf.ScaleX, tf.ScaleY,
		tf.Angle, target->GetHandle(), TRUE, FALSE);
}

void STextSprite::DrawScroll(const Transform2D & tf, const ColorTint & ct)
{
	const auto pds = GetDrawScreen();
	SetDrawScreen(scrollBuffer->GetHandle());
	SetDrawBright(255, 255, 255);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	ClearDrawScreen();
	if (scrollSpeed >= 0) {
		auto reach = -scrollPosition + int(scrollPosition / (get<0>(size) + scrollMargin)) * (get<0>(size) + scrollMargin);
		while (reach < scrollWidth) {
			DrawRectGraphF(
				SU_TO_FLOAT(reach), 0,
				0, 0, SU_TO_INT32(get<0>(size)), SU_TO_INT32(get<1>(size)),
				target->GetHandle(), TRUE, FALSE);
			reach += get<0>(size) + scrollMargin;
		}
	}
	else {
		auto reach = -scrollPosition - int(scrollPosition / (get<0>(size) + scrollMargin)) * (get<0>(size) + scrollMargin);
		while (reach > 0) {
			DrawRectGraphF(
				SU_TO_FLOAT(reach), 0,
				0, 0, SU_TO_INT32(get<0>(size)), SU_TO_INT32(get<1>(size)),
				target->GetHandle(), TRUE, FALSE);
			reach -= get<0>(size) + scrollMargin;
		}
	}
	SetDrawScreen(pds);

	if (isRich) {
		SetDrawBright(255, 255, 255);
	}
	else {
		SetDrawBright(ct.R, ct.G, ct.B);
	}
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ct.A);
	SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
	const auto tox = SU_TO_FLOAT(scrollWidth / 2 * int(horizontalAlignment));
	const auto toy = SU_TO_FLOAT(get<1>(size) / 2 * int(verticalAlignment));
	DrawRotaGraph3F(
		tf.X, tf.Y,
		tf.OriginX + tox, tf.OriginY + toy,
		tf.ScaleX, tf.ScaleY,
		tf.Angle, scrollBuffer->GetHandle(), TRUE, FALSE);
}

void STextSprite::SetFont(SFont * font)
{
	if (Font) Font->Release();
	Font = font;
	Refresh();
}

void STextSprite::SetText(const string & txt)
{
	Text = txt;
	Refresh();
}

void STextSprite::SetAlignment(const STextAlign hori, const STextAlign vert)
{
	horizontalAlignment = hori;
	verticalAlignment = vert;
}

void STextSprite::SetRangeScroll(const int width, const int margin, const double pps)
{
	isScrolling = width > 0;
	scrollWidth = width;
	scrollMargin = margin;
	scrollSpeed = pps;
	scrollPosition = 0;
	Refresh();
}

void STextSprite::SetRich(const bool enabled)
{
	isRich = enabled;
	Refresh();
}

double STextSprite::GetWidth()
{
	return get<0>(size);
}

double STextSprite::GetHeight()
{
	return get<1>(size);
}

STextSprite::~STextSprite()
{
	if (Font) Font->Release();
	delete target;
	delete scrollBuffer;
}

void STextSprite::Tick(const double delta)
{
	SSprite::Tick(delta);
	if (isScrolling) scrollPosition += scrollSpeed * delta;
}

void STextSprite::Draw()
{
	if (!target) return;
	if (isScrolling && target->GetWidth() >= scrollWidth) {
		DrawScroll(Transform, Color);
	}
	else {
		DrawNormal(Transform, Color);
	}
}

void STextSprite::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	if (!target) return;
	if (isScrolling && target->GetWidth() >= scrollWidth) {
		DrawScroll(tf, cl);
	}
	else {
		DrawNormal(tf, cl);
	}
}

STextSprite::STextSprite()
	: target(nullptr)
	, scrollBuffer(nullptr)
	, size(0.0, 0.0, 0)
	, horizontalAlignment(STextAlign::Left)
	, verticalAlignment(STextAlign::Top)
	, isScrolling(false)
	, scrollWidth(0)
	, scrollMargin(0)
	, scrollSpeed(0.0)
	, scrollPosition(0.0)
	, isRich(false)
	, Font(nullptr)
	, Text("")
{
}

STextSprite* STextSprite::Clone()
{
	//やっぱりコピコンで良くないかこれ
	auto clone = new STextSprite();
	clone->AddRef();

	clone->CopyParameterFrom(this);

	if (target) {
		clone->SetText(Text);
	}
	if (Font) {
		Font->AddRef();
		clone->SetFont(Font);
	}

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

STextSprite* STextSprite::Factory()
{
	auto result = new STextSprite();
	result->AddRef();

	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

STextSprite* STextSprite::Factory(SFont * img, const string & str)
{
	auto result = new STextSprite();
	result->AddRef();

	result->SetText(str);
	{
		if (img) img->AddRef();
		result->SetFont(img);
	}

	if (img) img->Release();
	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

void STextSprite::RegisterType(asIScriptEngine * engine)
{
	RegisterSpriteBasic<STextSprite>(engine, SU_IF_TXTSPRITE);
	engine->RegisterObjectBehaviour(SU_IF_TXTSPRITE, asBEHAVE_FACTORY, SU_IF_TXTSPRITE "@ f()", asFUNCTIONPR(STextSprite::Factory, (), STextSprite*), asCALL_CDECL);
	engine->RegisterObjectBehaviour(SU_IF_TXTSPRITE, asBEHAVE_FACTORY, SU_IF_TXTSPRITE "@ f(" SU_IF_FONT "@, const string &in)", asFUNCTIONPR(STextSprite::Factory, (SFont*, const string&), STextSprite*), asCALL_CDECL);
	engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_TXTSPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, STextSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<STextSprite, SSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetFont(" SU_IF_FONT "@)", asMETHOD(STextSprite, SetFont), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetText(const string &in)", asMETHOD(STextSprite, SetText), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetAlignment(" SU_IF_TEXTALIGN ", " SU_IF_TEXTALIGN ")", asMETHOD(STextSprite, SetAlignment), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetRangeScroll(int, int, double)", asMETHOD(STextSprite, SetRangeScroll), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "void SetRich(bool)", asMETHOD(STextSprite, SetRich), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "double get_Width()", asMETHOD(STextSprite, GetWidth), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_TXTSPRITE, "double get_Height()", asMETHOD(STextSprite, GetHeight), asCALL_THISCALL);
}

// STextInput ---------------------------------------

STextInput::STextInput()
{
	//TODO: デフォルト値の引数化
	inputHandle = MakeKeyInput(1024, TRUE, FALSE, FALSE, FALSE, FALSE);
}

STextInput::~STextInput()
{
	if (inputHandle) DeleteKeyInput(inputHandle);
}

void STextInput::SetFont(SFont * sfont)
{
	if (font) font->Release();
	font = sfont;
}

void STextInput::Activate() const
{
	SetActiveKeyInput(inputHandle);
}

void STextInput::Draw()
{
	const auto wcc = MultiByteToWideChar(CP_OEMCP, 0, currentRawString.c_str(), 1024, nullptr, 0);
	const auto widestr = new wchar_t[wcc];
	MultiByteToWideChar(CP_OEMCP, 0, currentRawString.c_str(), 1024, widestr, 0);
	delete[] widestr;
}

void STextInput::Tick(double delta)
{
	TCHAR buffer[1024] = { 0 };
	switch (CheckKeyInput(inputHandle)) {
	case 0:
		GetKeyInputString(buffer, inputHandle);
		currentRawString = buffer;
		cursor = GetKeyInputCursorPosition(inputHandle);
		GetKeyInputSelectArea(&selectionStart, &selectionEnd, inputHandle);
		return;
	case 1:
	case 2:
		selectionStart = selectionEnd = cursor = -1;
		return;
	default: break;
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
std::string STextInput::GetUTF8String() const
{
	return std::string();
}

STextInput* STextInput::Factory()
{
	return nullptr;
}

STextInput* STextInput::Factory(SFont * img)
{
	return nullptr;
}

void STextInput::RegisterType(asIScriptEngine * engine)
{}


// SSynthSprite -------------------------------------

void SSynthSprite::DrawBy(const Transform2D & tf, const ColorTint & ct)
{
	if (!target) return;
	SetDrawBright(ct.R, ct.G, ct.B);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ct.A);
	DrawRotaGraph3F(
		tf.X, tf.Y,
		tf.OriginX, tf.OriginY,
		tf.ScaleX, tf.ScaleY,
		tf.Angle, target->GetHandle(),
		HasAlpha ? TRUE : FALSE, FALSE);
}

SSynthSprite::SSynthSprite(const int w, const int h)
	: target(nullptr)
	, width(w)
	, height(h)
{
}

SSynthSprite::~SSynthSprite()
{
	delete target;
}

void SSynthSprite::Clear()
{
	delete target;
	target = new SRenderTarget(width, height);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SSynthSprite::Transfer(SSprite * sprite)
{
	if (!sprite) return;

	BEGIN_DRAW_TRANSACTION(target->GetHandle());
	sprite->Draw();
	FINISH_DRAW_TRANSACTION;

	sprite->Release();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void SSynthSprite::Transfer(SImage * image, const double x, const double y)
{
	if (!image) return;

	BEGIN_DRAW_TRANSACTION(target->GetHandle());
	SetDrawBright(255, 255, 255);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	DrawGraphF(SU_TO_FLOAT(x), SU_TO_FLOAT(y), image->GetHandle(), HasAlpha ? TRUE : FALSE);
	FINISH_DRAW_TRANSACTION;

	image->Release();
}

void SSynthSprite::Draw()
{
	DrawBy(Transform, Color);
}

void SSynthSprite::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	DrawBy(tf, cl);
}

SSynthSprite* SSynthSprite::Clone()
{
	auto clone = new SSynthSprite(width, height);
	clone->AddRef();

	clone->CopyParameterFrom(this);

	if (target) {
		this->AddRef();
		clone->Transfer(this);
	}

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

SSynthSprite* SSynthSprite::Factory(const int w, const int h)
{
	auto result = new SSynthSprite(w, h);
	result->AddRef();

	result->Clear();

	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

void SSynthSprite::RegisterType(asIScriptEngine * engine)
{
	RegisterSpriteBasic<SSynthSprite>(engine, SU_IF_SYHSPRITE);
	engine->RegisterObjectBehaviour(SU_IF_SYHSPRITE, asBEHAVE_FACTORY, SU_IF_SYHSPRITE "@ f(int, int)", asFUNCTION(SSynthSprite::Factory), asCALL_CDECL);
	engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_SYHSPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SSynthSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_SYHSPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SSynthSprite, SSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "int get_Width()", asMETHOD(SSynthSprite, GetWidth), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "int get_Height()", asMETHOD(SSynthSprite, GetWidth), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "void Clear()", asMETHOD(SSynthSprite, Clear), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "void Transfer(" SU_IF_SPRITE "@)", asMETHODPR(SSynthSprite, Transfer, (SSprite*), void), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_SYHSPRITE, "void Transfer(" SU_IF_IMAGE "@, double, double)", asMETHODPR(SSynthSprite, Transfer, (SImage*, double, double), void), asCALL_THISCALL);
}

// SClippingSprite ------------------------------------------

void SClippingSprite::DrawBy(const Transform2D & tf, const ColorTint & ct)
{
	if (!target) return;
	const auto x = SU_TO_INT32(width * u1);
	const auto y = SU_TO_INT32(height * v1);
	const auto w = SU_TO_INT32(width * u2);
	const auto h = SU_TO_INT32(height * v2);
	SetDrawBright(ct.R, ct.G, ct.B);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, ct.A);
	DrawRectRotaGraph3F(
		tf.X, tf.Y,
		x, y, w, h,
		tf.OriginX, tf.OriginY,
		tf.ScaleX, tf.ScaleY,
		tf.Angle, target->GetHandle(),
		HasAlpha ? TRUE : FALSE, FALSE);
}

SClippingSprite::SClippingSprite(const int w, const int h)
	: SSynthSprite(w, h)
	, u1(0), v1(0)
	, u2(1), v2(0)
{}

void SClippingSprite::SetRange(const double tx, const double ty, const double w, const double h)
{
	u1 = tx;
	v1 = ty;
	u2 = w;
	v2 = h;
}

void SClippingSprite::Draw()
{
	DrawBy(Transform, Color);
}

void SClippingSprite::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	DrawBy(tf, cl);
}

SClippingSprite* SClippingSprite::Clone()
{
	auto clone = new SClippingSprite(width, height);
	clone->AddRef();

	clone->CopyParameterFrom(this);

	if (target) {
		this->AddRef();
		clone->Transfer(this);
	}

	clone->u1 = u1;
	clone->v1 = v1;
	clone->u2 = u2;
	clone->v2 = v2;

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

SClippingSprite* SClippingSprite::Factory(const int w, const int h)
{
	auto result = new SClippingSprite(w, h);
	result->AddRef();

	result->Clear();

	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

void SClippingSprite::RegisterType(asIScriptEngine * engine)
{
	RegisterSpriteBasic<SClippingSprite>(engine, SU_IF_CLPSPRITE);
	engine->RegisterObjectBehaviour(SU_IF_CLPSPRITE, asBEHAVE_FACTORY, SU_IF_CLPSPRITE "@ f(int, int)", asFUNCTION(SClippingSprite::Factory), asCALL_CDECL);
	engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_CLPSPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SClippingSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SClippingSprite, SSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "int get_Width()", asMETHOD(SClippingSprite, GetWidth), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "int get_Height()", asMETHOD(SClippingSprite, GetWidth), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void Clear()", asMETHOD(SClippingSprite, Clear), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void Transfer(" SU_IF_SPRITE "@)", asMETHODPR(SClippingSprite, Transfer, (SSprite*), void), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void Transfer(" SU_IF_IMAGE "@, double, double)", asMETHODPR(SClippingSprite, Transfer, (SImage*, double, double), void), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_CLPSPRITE, "void SetRange(double, double, double, double)", asMETHOD(SClippingSprite, SetRange), asCALL_THISCALL);
}

// SAnimeSprite -------------------------------------------
void SAnimeSprite::DrawBy(const Transform2D & tf, const ColorTint & ct)
{
	const auto at = images->GetCellTime() * images->GetFrameCount() - time;
	const auto ih = images->GetImageHandleAt(at);
	if (!ih) return;
	SetDrawBright(Color.R, Color.G, Color.B);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, Color.A);
	DrawRotaGraph3F(
		Transform.X, Transform.Y,
		Transform.OriginX, Transform.OriginY,
		Transform.ScaleX, Transform.ScaleY,
		Transform.Angle, ih,
		HasAlpha ? TRUE : FALSE, FALSE);
}

SAnimeSprite::SAnimeSprite(SAnimatedImage * img)
	: images(img)
	, loopCount(1)
	, count(0)
	, speed(1)
	, time(img ? (img->GetCellTime() * img->GetFrameCount()) : 0)
{
}

SAnimeSprite::~SAnimeSprite()
{
	if (images) images->Release();
}

void SAnimeSprite::Draw()
{
	DrawBy(Transform, Color);
}

void SAnimeSprite::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	DrawBy(tf, cl);
}

void SAnimeSprite::Tick(const double delta)
{
	pMover->Tick(delta);

	time -= delta * speed;
	if (time > 0) return;
	if (loopCount > 0 && ++count == loopCount) Dismiss();
	time = images->GetCellTime() * images->GetFrameCount();
}

SAnimeSprite* SAnimeSprite::Clone()
{
	if (images) images->AddRef();
	auto clone = new SAnimeSprite(images);
	clone->AddRef();

	clone->CopyParameterFrom(this);

	clone->loopCount = loopCount;
	clone->speed = speed;

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

SAnimeSprite* SAnimeSprite::Factory(SAnimatedImage * image)
{
	if (image) image->AddRef();
	auto result = new SAnimeSprite(image);
	result->AddRef();

	if (image) image->Release();
	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

void SAnimeSprite::RegisterType(asIScriptEngine * engine)
{
	RegisterSpriteBasic<SAnimeSprite>(engine, SU_IF_ANIMESPRITE);
	engine->RegisterObjectBehaviour(SU_IF_ANIMESPRITE, asBEHAVE_FACTORY, SU_IF_ANIMESPRITE "@ f(" SU_IF_ANIMEIMAGE "@)", asFUNCTIONPR(SAnimeSprite::Factory, (SAnimatedImage*), SAnimeSprite*), asCALL_CDECL);
	engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_ANIMESPRITE "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SAnimeSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_ANIMESPRITE, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SAnimeSprite, SSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_ANIMESPRITE, "void SetSpeed(double)", asMETHOD(SAnimeSprite, SetSpeed), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_ANIMESPRITE, "void SetLoopCount(int)", asMETHOD(SAnimeSprite, SetLoopCount), asCALL_THISCALL);
}

SContainer::SContainer() : SSprite()
{}

SContainer::~SContainer()
{
	for (const auto& s : children) if (s) s->Release();
}

void SContainer::AddChild(SSprite * child)
{
	if (!child) return;
	children.emplace(child);
}

void SContainer::Dismiss()
{
	for (const auto& child : children) {
		child->Dismiss();
	}
	SSprite::Dismiss();
}

void SContainer::Tick(const double delta)
{
	pMover->Tick(delta);
	for (const auto& s : children) s->Tick(delta);
}

void SContainer::Draw()
{
	for (const auto& s : children) s->Draw(Transform, Color);
}

void SContainer::Draw(const Transform2D & parent, const ColorTint & color)
{
	const auto tf = Transform.ApplyFrom(parent);
	const auto cl = Color.ApplyFrom(color);
	for (const auto& s : children) s->Draw(tf, cl);
}

SContainer* SContainer::Clone()
{
	auto clone = new SContainer();
	clone->AddRef();

	clone->CopyParameterFrom(this);
	for (const auto& s : children) if (s) clone->AddChild(s->Clone());

	BOOST_ASSERT(clone->GetRefCount() == 1);
	return clone;
}

SContainer * SContainer::Factory()
{
	auto result = new SContainer();
	result->AddRef();

	BOOST_ASSERT(result->GetRefCount() == 1);
	return result;
}

void SContainer::RegisterType(asIScriptEngine * engine)
{
	RegisterSpriteBasic<SContainer>(engine, SU_IF_CONTAINER);
	engine->RegisterObjectBehaviour(SU_IF_CONTAINER, asBEHAVE_FACTORY, SU_IF_CONTAINER "@ f()", asFUNCTION(SContainer::Factory), asCALL_CDECL);
	engine->RegisterObjectMethod(SU_IF_SPRITE, SU_IF_CONTAINER "@ opCast()", asFUNCTION((CastReferenceType<SSprite, SContainer>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_CONTAINER, SU_IF_SPRITE "@ opImplCast()", asFUNCTION((CastReferenceType<SContainer, SSprite>)), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(SU_IF_CONTAINER, "void AddChild(" SU_IF_SPRITE "@)", asMETHOD(SContainer, AddChild), asCALL_THISCALL);
}
