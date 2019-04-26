#include "Misc.h"
#include "ScriptSpriteMover.h"
#include "ScriptSprite.h"
#include "MoverFunctionExpression.h"

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

bool MoverObject::RegisterType(asIScriptEngine * engine)
{
	engine->RegisterObjectType(SU_IF_MOVER_OBJECT, 0, asOBJ_REF);
	engine->RegisterObjectBehaviour(SU_IF_MOVER_OBJECT, asBEHAVE_ADDREF, "void f()", asMETHOD(MoverObject, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_MOVER_OBJECT, asBEHAVE_RELEASE, "void f()", asMETHOD(MoverObject, Release), asCALL_THISCALL);
	engine->RegisterObjectBehaviour(SU_IF_MOVER_OBJECT, asBEHAVE_FACTORY, SU_IF_MOVER_OBJECT "@ f()", asFUNCTIONPR(MoverObject::Factory, (), MoverObject*), asCALL_CDECL);

	engine->RegisterObjectMethod(SU_IF_MOVER_OBJECT, SU_IF_MOVER_OBJECT "@ Clone()", asMETHOD(MoverObject, Clone), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MOVER_OBJECT, "void Clear()", asMETHOD(MoverObject, Clear), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MOVER_OBJECT, "bool Apply(const string &in)", asMETHODPR(MoverObject, Apply, (const string&), bool), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MOVER_OBJECT, "bool Apply(const string &in, double)", asMETHODPR(MoverObject, Apply, (const string&, double), bool), asCALL_THISCALL);
	engine->RegisterObjectMethod(SU_IF_MOVER_OBJECT, "bool Apply(const string &in, const string &in)", asMETHODPR(MoverObject, Apply, (const string&, const string&), bool), asCALL_THISCALL);

	return true;
}

MoverObject* MoverObject::Factory()
{
	return new MoverObject();
}

MoverObject* MoverObject::Clone()
{
	MoverObject* pObj = new MoverObject();

	pObj->pFunction = pFunction;

	pObj->time = time;
	pObj->wait = wait;
	pObj->begin = begin;
	pObj->end = end;
	pObj->isBeginOffset = isBeginOffset;
	pObj->isEndOffset = isEndOffset;

	return pObj;
}

void MoverObject::Clear()
{
	time = 0.0;
	wait = 0.0;
	begin = Values::Default;
	end = Values::Default;
	isBeginOffset = false;
	isEndOffset = false;
}

bool MoverObject::InitVariables()
{
	BOOST_ASSERT(target);
	if (!pFunction) return false;

	double value;
	if (!SSprite::GetField(target, fieldID, value)) return false;

	variables.Begin = !isfinite(begin) ? value : (isBeginOffset ? value + begin : begin);
	variables.End = !isfinite(end) ? value : (isEndOffset ? value + end : end);
	variables.Diff = variables.End - variables.Begin;
	variables.Progress = 0.0;
	variables.Current = 0.0;

	state = StateID::Working;
	return true;
}

bool MoverObject::Apply(const string & dict)
{
	const char* p = dict.c_str();
	return ::AddMoveObject(dict.c_str(), &p, this) && p == '\0';
}

bool MoverObject::Apply(const string & key, double value)
{
	switch (Crc32Rec(0xFFFFFFFF, key.c_str())) {
	case "begin"_crc32:
		begin = value;
		isBeginOffset = false;
		break;
	case "@begin"_crc32:
		begin = value;
		isBeginOffset = true;
		break;
	case "end"_crc32:
		end = value;
		isEndOffset = false;
		break;
	case "@end"_crc32:
		end = value;
		isEndOffset = true;
		break;
	case "time"_crc32:
		time = value;
		break;
	case "wait"_crc32:
		wait = value;
		break;
	default:
		spdlog::get("main")->warn(u8"SpriteMover に実数引数を持つプロパティ \"{0}\" は設定できません。", key);
		return false;
	}
	return true;
}

bool MoverObject::Apply(const string & key, const string & value)
{
	switch (Crc32Rec(0xFFFFFFFF, key.c_str())) {
	case "func"_crc32:
	{
		MoverFunctionExpressionSharedPtr pFunc;
		if (!MoverFunctionExpressionManager::GetInstance().Find(value, pFunc) || !pFunc) {
			spdlog::get("main")->error(u8"キー \"{0}\" に対応する MoverFunction が見つかりませんでした。", value);
			return false;
		}
		pFunction = pFunc;
		break;
	}
	default:
		spdlog::get("main")->warn(u8"SpriteMover に 文字列引数を持つプロパティ \"{0}\" は設定できません。", key);
		return false;
	}
	return true;
}

bool MoverObject::Tick(double delta)
{
	BOOST_ASSERT(target);

	if (state == StateID::Suspend) {
		if (wait <= 0) {
			return InitVariables();
		}

		state = StateID::Waiting;
		waiting = 0;
		return true;
	}
	else if (state == StateID::Waiting) {
		waiting += delta;
		if (waiting < wait) {
			return true;
		}

		return InitVariables();
	}
	else if (state == StateID::Working) {
		variables.Current += delta;

		if (variables.Current > time) {
			variables.Current = time;
			variables.Progress = 1.0;
		}
		else {
			variables.Progress = variables.Current / time;
		}

		if (variables.Progress >= 1.0) {
			state = StateID::Done;
		}

		return true;
	}

	return false;
}

bool MoverObject::Abort()
{
	if (!(state == StateID::Working || state == StateID::Done)) {
		if (!InitVariables()) return false;
	}

	variables.Current = time;
	variables.Progress = 1.0;

	return SSprite::SetField(target, fieldID, pFunction->Execute(variables));
}


asUINT SSpriteMover::StrTypeId = 0;

void SSpriteMover::Tick(const double delta)
{
	BOOST_ASSERT(target);

	auto it = moves.begin();
	while (it != moves.end()) {
		auto& pMover = *it;
		if (!pMover->Tick(delta)) {
			// TODO: ログ
			spdlog::get("main")->error(u8"Tick処理に失敗");

			continue;
		}

		const auto state = pMover->GetState();
		if (state == MoverObject::StateID::Working || state == MoverObject::StateID::Done) {
			if (!pMover->Execute()) {
				// TODO: ログ
				spdlog::get("main")->error(u8"Mover死す");

				pMover->Release();
				it = moves.erase(it);
				continue;
			}

			if (state == MoverObject::StateID::Done) {
				pMover->Release();
				it = moves.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			if (state == MoverObject::StateID::Waiting) {
				++it;
			}
			else {
				// Note: 本来ここには入らないはずだけど……
				pMover->Release();
				it = moves.erase(it);
			}
		}
	}

	if (target->IsDead) {
		Abort(false);
	}
}

bool SSpriteMover::AddMove(const std::string & dict)
{
	if (!::AddMove(dict.c_str(), this)) {
		spdlog::get("main")->warn(u8"AddMoveのパースに失敗しました。 : \"{0}\"", dict);
		return false;
	}

	return true;
}

bool SSpriteMover::AddMove(const std::string & key, const CScriptDictionary * dict)
{
	BOOST_ASSERT(target);
	MoverObject* pMover = new MoverObject();

	pMover->RegisterTargetField(SSprite::GetFieldId(key));

	auto i = dict->begin();
	while (i != dict->end()) {
		const auto key = i.GetKey();
		double dv = 0;
		string sv;

		const auto StrTypeID = SSpriteMover::StrTypeId;
		if (i.GetTypeId() == StrTypeID && i.GetValue(&sv, StrTypeID)) {
			if (!pMover->Apply(key, sv)) {
				spdlog::get("main")->warn(u8"キー \"{0}\" にする値 \"{1}\" (as string) の設定に失敗しました。", key, sv);
			}
		}
		else if (i.GetValue(dv)) {
			if (!pMover->Apply(key, dv)) {
				spdlog::get("main")->warn(u8"キー \"{0}\" にする値 \"{1}\" (as double) の設定に失敗しました。", key, dv);
			}
		}
		else {
			spdlog::get("main")->warn(u8"キー \"{0}\" に対し有効な値が設定できませんでした。", key);
		}

		++i;
	}

	dict->Release();

	return AddMove(pMover);
}

bool SSpriteMover::AddMove(MoverObject * pMover)
{
	if (!pMover) return false;

	if (!pMover->HasFunction()) {
		if (!pMover->Apply("func", "linear")) {
			pMover->Release();
			return false;
		}
	}

	if (!pMover->SetTargetSprite(target)) {
		pMover->Release();
		return false;
	}

	moves.emplace_back(pMover);
	return true;
}

void SSpriteMover::Abort(const bool completeMove)
{
	for (const auto& it : moves) {
		if (completeMove) {
			if (!it->Abort()) {
				// TODO: ログ
			}
		}
		it->Release();
	}

	moves.clear();
}

