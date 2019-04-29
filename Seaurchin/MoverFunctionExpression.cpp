#define _USE_MATH_DEFINES
#include <iostream>
#include <memory>
#include <cmath>
#include <vector>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

using namespace crc32_constexpr;


namespace {
	std::random_device rnd;
	std::mt19937 mt(rnd());

	double rand(double min, double max) {
		if (min > max) {
			const double t = min;
			min = max;
			max = t;
		}
		const double r = SU_TO_DOUBLE(mt()) / SU_TO_DOUBLE(mt.max());
		const double d = max - min;
		return min + r * d;
	}
}

#include "MoverFunctionExpression.h"

#define SU_DEF_VARIABLE_MOVER_FUNCTION_EXPRESSION(name, value) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
public: \
	double Execute(const MoverFunctionExpressionVariables& var) const override { return var.value; } \
}

SU_DEF_VARIABLE_MOVER_FUNCTION_EXPRESSION(Begin, Begin);
SU_DEF_VARIABLE_MOVER_FUNCTION_EXPRESSION(End, End);
SU_DEF_VARIABLE_MOVER_FUNCTION_EXPRESSION(Diff, Diff);
SU_DEF_VARIABLE_MOVER_FUNCTION_EXPRESSION(Current, Current);
SU_DEF_VARIABLE_MOVER_FUNCTION_EXPRESSION(Progress, Progress);

class LiteralMoverFunctionExpression : public MoverFunctionExpression
{
private:
	double value;
public:
	LiteralMoverFunctionExpression(double value) : value(value) {}
	double Execute(const MoverFunctionExpressionVariables& var) const override { return value; }
};

#define SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(name, value) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
public: \
	double Execute(const MoverFunctionExpressionVariables& var) const override { return value; } \
}

SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(E, M_E);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(LOG2E, M_LOG2E);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(LOG10E, M_LOG10E);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(LN2, M_LN2);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(LN10, M_LN10);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(PI, M_PI);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(PI_2, M_PI_2);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(PI_4, M_PI_4);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(INV_PI, M_1_PI);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(INV_PI_2, M_2_PI);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(INV_SQRTPI_2, M_2_SQRTPI);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(SQRT2, M_SQRT2);
SU_DEF_CONST_LITERAL_MOVER_FUNCTION_EXPRESSION(INV_SQRT2, M_SQRT1_2);

#define SU_DEF_SINGLE_OPERAND_MOVER_FUNCTION_EXPRESSION(name, op) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
private: \
	MoverFunctionExpressionSharedPtr pOp; \
public: \
	name ## MoverFunctionExpression(MoverFunctionExpressionSharedPtr &pOp) : pOp(pOp) {} \
	double Execute(const MoverFunctionExpressionVariables& var) const override { return op pOp->Execute(var); } \
}

SU_DEF_SINGLE_OPERAND_MOVER_FUNCTION_EXPRESSION(Positive, +);
SU_DEF_SINGLE_OPERAND_MOVER_FUNCTION_EXPRESSION(Negative, -);

#define SU_DEF_DOUBLE_OPERAND_MOVER_FUNCTION_EXPRESSION(name, op) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
private: \
	MoverFunctionExpressionSharedPtr pLop, pRop; \
public: \
	name ## MoverFunctionExpression(MoverFunctionExpressionSharedPtr &pLop, MoverFunctionExpressionSharedPtr &pRop) : pLop(pLop), pRop(pRop) {} \
	double Execute(const MoverFunctionExpressionVariables& var) const override { return pLop->Execute(var) op pRop->Execute(var); } \
}

SU_DEF_DOUBLE_OPERAND_MOVER_FUNCTION_EXPRESSION(Add, +);
SU_DEF_DOUBLE_OPERAND_MOVER_FUNCTION_EXPRESSION(Sub, -);
SU_DEF_DOUBLE_OPERAND_MOVER_FUNCTION_EXPRESSION(Mul, *);
SU_DEF_DOUBLE_OPERAND_MOVER_FUNCTION_EXPRESSION(Div, / );

#define SU_DEF_DOUBLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(name, func) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
private: \
	MoverFunctionExpressionSharedPtr pLop, pRop; \
public: \
	name ## MoverFunctionExpression(MoverFunctionExpressionSharedPtr &pLop, MoverFunctionExpressionSharedPtr &pRop) : pLop(pLop), pRop(pRop) {} \
	double Execute(const MoverFunctionExpressionVariables& var) const override { return func (pLop->Execute(var), pRop->Execute(var)); } \
}

SU_DEF_DOUBLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Mod, fmod);
SU_DEF_DOUBLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Pow, pow);
SU_DEF_DOUBLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Min, fmin);
SU_DEF_DOUBLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Max, fmax);
SU_DEF_DOUBLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Rand, rand);

#define SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(name, func) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
private: \
	MoverFunctionExpressionSharedPtr pArg; \
public: \
	name ## MoverFunctionExpression(MoverFunctionExpressionSharedPtr &pArg) : pArg(pArg) {} \
	double Execute(const MoverFunctionExpressionVariables& var) const override { return func (pArg->Execute(var)); } \
}

SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Abs, fabs);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Round, round);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Ceil, ceil);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Floor, floor);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Exp, exp);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Ln, log);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Log, log10);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Sin, sin);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Cos, cos);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Tan, tan);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Asin, asin);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Acos, acos);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Atan, atan);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Sinh, sinh);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Cosh, cosh);
SU_DEF_SINGLE_ARGUMENT_MOVER_FUNCTION_EXPRESSION(Tanh, tanh);

template<typename T>
MoverFunctionExpressionSharedPtr MakeExp()
{
	return std::make_shared<T>();
}

template<typename T>
MoverFunctionExpressionSharedPtr MakeExp_double(double val)
{
	return std::make_shared<T>(val);
}

template<typename T>
MoverFunctionExpressionSharedPtr MakeExp_Ptr(MoverFunctionExpressionSharedPtr lop)
{
	return std::make_shared<T>(lop);
}

template<typename T>
MoverFunctionExpressionSharedPtr MakeExp_Ptr_Ptr(MoverFunctionExpressionSharedPtr lop, MoverFunctionExpressionSharedPtr rop)
{
	return std::make_shared<T>(lop, rop);
}

static bool GetExpression(MoverFunctionExpressionSharedPtr& ret, const char* expression, const char** seeked);
static bool GetNode(MoverFunctionExpressionSharedPtr& ret, const char* expression, const char** seeked);

bool GetExpression(MoverFunctionExpressionSharedPtr& ret, const char* expression, const char** seeked)
{
	std::cout << expression << std::endl;
	ret.reset();

	MoverFunctionExpressionSharedPtr op;

#define SKIP_SP() while(!isgraph(*p)) ++p
	const char* p = expression;
	bool isNesting = false;

	SKIP_SP();
	if (*p == '(') {
		isNesting = true;
		++p;

		const char* next = p;

		if (!GetExpression(op, p, &next)) return false;
		p = next;
	}
	else {
		const char* next = p;
		if (!GetNode(op, p, &next)) return false;
		p = next;
	}

	while (*p != '\0') {
		SKIP_SP();
		if (*p == '+') {
			if (*(p + 1) == '+') return false;
			++p;

			const char* next = p;
			MoverFunctionExpressionSharedPtr rop;
			if (!GetExpression(rop, p, &next)) return false;
			p = next;

			op.reset(new AddMoverFunctionExpression(op, rop));
			continue;
		}
		else if (*p == '-') {
			if (*(p + 1) == '-') return false;
			++p;

			const char* next = p;
			MoverFunctionExpressionSharedPtr rop;
			if (!GetExpression(rop, p, &next)) return false;
			p = next;

			op.reset(new SubMoverFunctionExpression(op, rop));
			continue;
		}
		else if (*p == '*') {
			++p;

			const char* next = p;
			MoverFunctionExpressionSharedPtr rop;
			if (!GetNode(rop, p, &next)) return false;
			p = next;

			op.reset(new MulMoverFunctionExpression(op, rop));
			continue;
		}
		else if (*p == '/') {
			++p;

			const char* next = p;
			MoverFunctionExpressionSharedPtr rop;
			if (!GetNode(rop, p, &next)) return false;
			p = next;

			op.reset(new DivMoverFunctionExpression(op, rop));
			continue;
		}
		else if (*p == '%') {
			++p;

			const char* next = p;
			MoverFunctionExpressionSharedPtr rop;
			if (!GetNode(rop, p, &next)) return false;
			p = next;

			op.reset(new ModMoverFunctionExpression(op, rop));
			continue;
		}
		else if (*p == '^') {
			++p;

			const char* next = p;
			MoverFunctionExpressionSharedPtr rop;
			if (!GetNode(rop, p, &next)) return false;
			p = next;

			op.reset(new PowMoverFunctionExpression(op, rop));
			continue;
		}
		else if (*p == ')') {
			if (isNesting) ++p;

			break;
		}
		else {
			break;
		}
	}
#undef SKIP_SP


	if (!op) return false;

	ret = op;
	*seeked = p;

	return true;
}

bool GetNode(MoverFunctionExpressionSharedPtr & ret, const char* expression, const char** seeked)
{
	ret.reset();

#define SKIP_SP() while(!isgraph(*p)) ++p
	const char* p = expression;
	if (*p == '\0') return false;

	SKIP_SP();

	bool isPositive = false, isNegative = false;
	if (*p == '+') {
		if (*(p + 1) == '+') return false;
		isPositive = true;
		++p;
		SKIP_SP();
	}
	else if (*p == '-') {
		if (*(p + 1) == '-') return false;
		isNegative = true;
		++p;
		SKIP_SP();
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

		ret.reset(new LiteralMoverFunctionExpression(val));
		*seeked = p;
		return true;
	}
	else if (isalpha(*p) || *p == '_') {
		const char* head = p;
		do ++p; while (isalnum(*p) || *p == '_');
		const char* tail = p;
		const size_t len = tail - head;

		bool retval = false;
		{
			char* buf = (char*)malloc(sizeof(char) * (len + 1));
			if (buf == NULL) return false;

			memcpy(buf, head, len);
			buf[len] = '\0';

			switch (Crc32Rec(0xFFFFFFFF, buf)) {
			case "add"_crc32:
			case "sub"_crc32:
			case "mul"_crc32:
			case "div"_crc32:
			case "mod"_crc32:
			case "pow"_crc32:
			case "min"_crc32:
			case "max"_crc32:
			case "rand"_crc32:
			{
				SKIP_SP();
				if (*p != '(') break;
				++p;

				MoverFunctionExpressionSharedPtr lop, rop;
				const char* next = p;
				if (!GetExpression(lop, p, &next)) break;
				p = next;

				SKIP_SP();
				if (*p != ',') break;
				++p;

				next = p;
				if (!GetExpression(rop, p, &next)) break;
				p = next;

				SKIP_SP();
				if (*p != ')') break;
				++p;

				retval = true;
				switch (Crc32Rec(0xFFFFFFFF, buf)) {
				case "add"_crc32:
					ret.reset(new AddMoverFunctionExpression(lop, rop));
					break;
				case "sub"_crc32:
					ret.reset(new SubMoverFunctionExpression(lop, rop));
					break;
				case "mul"_crc32:
					ret.reset(new MulMoverFunctionExpression(lop, rop));
					break;
				case "div"_crc32:
					ret.reset(new DivMoverFunctionExpression(lop, rop));
					break;
				case "mod"_crc32:
					ret.reset(new ModMoverFunctionExpression(lop, rop));
					break;
				case "pow"_crc32:
					ret.reset(new PowMoverFunctionExpression(lop, rop));
					break;
				case "min"_crc32:
					ret.reset(new MinMoverFunctionExpression(lop, rop));
					break;
				case "max"_crc32:
					ret.reset(new MaxMoverFunctionExpression(lop, rop));
					break;
				case "rand"_crc32:
					ret.reset(new RandMoverFunctionExpression(lop, rop));
					break;
				default:
					retval = false;
					break;
				}
				break;
			}
			case "abs"_crc32:
			case "round"_crc32:
			case "ceil"_crc32:
			case "floor"_crc32:
			case "exp"_crc32:
			case "ln"_crc32:
			case "log"_crc32:
			case "sin"_crc32:
			case "cos"_crc32:
			case "tan"_crc32:
			case "asin"_crc32:
			case "acos"_crc32:
			case "atan"_crc32:
			case "sinh"_crc32:
			case "cosh"_crc32:
			case "tanh"_crc32:
			{
				SKIP_SP();
				if (*p != '(') break;
				++p;

				MoverFunctionExpressionSharedPtr op;
				const char* next = p;
				if (!GetExpression(op, p, &next)) break;
				p = next;

				SKIP_SP();;
				if (*p != ')') break;
				++p;

				retval = true;
				switch (Crc32Rec(0xFFFFFFFF, buf)) {
				case "abs"_crc32:
					ret.reset(new AbsMoverFunctionExpression(op));
					break;
				case "round"_crc32:
					ret.reset(new RoundMoverFunctionExpression(op));
					break;
				case "ceil"_crc32:
					ret.reset(new CeilMoverFunctionExpression(op));
					break;
				case "floor"_crc32:
					ret.reset(new FloorMoverFunctionExpression(op));
					break;
				case "exp"_crc32:
					ret.reset(new ExpMoverFunctionExpression(op));
					break;
				case "ln"_crc32:
					ret.reset(new LnMoverFunctionExpression(op));
					break;
				case "log"_crc32:
					ret.reset(new LogMoverFunctionExpression(op));
					break;
				case "sin"_crc32:
					ret.reset(new SinMoverFunctionExpression(op));
					break;
				case "cos"_crc32:
					ret.reset(new CosMoverFunctionExpression(op));
					break;
				case "tan"_crc32:
					ret.reset(new TanMoverFunctionExpression(op));
					break;
				case "asin"_crc32:
					ret.reset(new AsinMoverFunctionExpression(op));
					break;
				case "acos"_crc32:
					ret.reset(new AcosMoverFunctionExpression(op));
					break;
				case "atan"_crc32:
					ret.reset(new AtanMoverFunctionExpression(op));
					break;
				case "sinh"_crc32:
					ret.reset(new SinhMoverFunctionExpression(op));
					break;
				case "cosh"_crc32:
					ret.reset(new CoshMoverFunctionExpression(op));
					break;
				case "tanh"_crc32:
					ret.reset(new TanhMoverFunctionExpression(op));
					break;
				default:
					retval = false;
					break;
				}
				break;
			}
			case "begin"_crc32:
				retval = true;
				ret.reset(new BeginMoverFunctionExpression());
				break;
			case "end"_crc32:
				retval = true;
				ret.reset(new EndMoverFunctionExpression());
				break;
			case "diff"_crc32:
				retval = true;
				ret.reset(new DiffMoverFunctionExpression());
				break;
			case "current"_crc32:
				retval = true;
				ret.reset(new CurrentMoverFunctionExpression());
				break;
			case "progress"_crc32:
				retval = true;
				ret.reset(new ProgressMoverFunctionExpression());
				break;
			case "e"_crc32:
				retval = true;
				ret.reset(new EMoverFunctionExpression());
				break;
			case "log2e"_crc32:
				retval = true;
				ret.reset(new LOG2EMoverFunctionExpression());
				break;
			case "log10e"_crc32:
				retval = true;
				ret.reset(new LOG10EMoverFunctionExpression());
				break;
			case "ln2"_crc32:
				retval = true;
				ret.reset(new LN2MoverFunctionExpression());
				break;
			case "ln10"_crc32:
				retval = true;
				ret.reset(new LN10MoverFunctionExpression());
				break;
			case "pi"_crc32:
				retval = true;
				ret.reset(new PIMoverFunctionExpression());
				break;
			case "pi_2"_crc32:
				retval = true;
				ret.reset(new PI_2MoverFunctionExpression());
				break;
			case "pi_4"_crc32:
				retval = true;
				ret.reset(new PI_4MoverFunctionExpression());
				break;
			case "inv_pi"_crc32:
				retval = true;
				ret.reset(new INV_PIMoverFunctionExpression());
				break;
			case "inv_pi_2"_crc32:
				retval = true;
				ret.reset(new INV_PI_2MoverFunctionExpression());
				break;
			case "inv_sqrtpi_2"_crc32:
				retval = true;
				ret.reset(new INV_SQRTPI_2MoverFunctionExpression());
				break;
			case "sqrt2"_crc32:
				retval = true;
				ret.reset(new SQRT2MoverFunctionExpression());
				break;
			case "inv_sqrt2"_crc32:
				retval = true;
				ret.reset(new INV_SQRT2MoverFunctionExpression());
				break;
			default:
				break;
			}

			free(buf);
		}

		if (retval) {
			if (isNegative) {
				ret.reset(new NegativeMoverFunctionExpression(ret));
			}
		}

		if (retval)* seeked = p;
		return retval;
	}
#undef SKIP_SP

	return false;
}

bool ParseMoverFunctionExpression(MoverFunctionExpressionSharedPtr & root, const std::string & expression)
{
	const char* p = expression.c_str();
	return GetExpression(root, expression.c_str(), &p) && (*p == '\0');
}


MoverFunctionExpressionManager* MoverFunctionExpressionManager::inst;

bool MoverFunctionExpressionManager::Initialize()
{
	BOOST_ASSERT(!inst);
	if (!!inst) return false;

	inst = new MoverFunctionExpressionManager();
	BOOST_ASSERT(!!inst);

	return !!inst;
}

bool MoverFunctionExpressionManager::Finalize()
{
	BOOST_ASSERT(!!inst);
	if (!inst) return false;

	delete inst;

	return !inst;
}

bool MoverFunctionExpressionManager::Register(const std::string & key, const std::string & expression)
{
	BOOST_ASSERT(!!inst);
	if (!inst) return false;

	MoverFunctionExpressionSharedPtr pFunction;
	if (!ParseMoverFunctionExpression(pFunction, expression) || !pFunction) {
		spdlog::get("main")->error(u8"\"{0}\" 関数 (\"{1}\") の登録に失敗しました。", key, expression);
		return false;
	}

	return GetInstance().Register(key, pFunction);
}

bool MoverFunctionExpressionManager::Register(const std::string & key, MoverFunctionExpression * pFunction)
{
	const MoverFunctionExpressionSharedPtr ptr(pFunction);
	return Register(key, ptr);
}

bool MoverFunctionExpressionManager::Register(const std::string & key, const MoverFunctionExpressionSharedPtr & pFunction)
{
	if (list.find(key) != list.end()) {
		spdlog::get("main")->warn(u8"\"{0}\" 関数は既に登録されています。", key);
		return false;
	}

	list.insert(std::make_pair(key, pFunction));

	return true;
}

bool MoverFunctionExpressionManager::IsRegistered(const std::string & key)
{
	BOOST_ASSERT(!!inst);
	if (!inst) return false;

	MoverFunctionExpressionSharedPtr pFunction;
	const bool retVal = GetInstance().Find(key, pFunction);
	return retVal && pFunction;
}

bool MoverFunctionExpressionManager::Find(const std::string & key, MoverFunctionExpressionSharedPtr & pFunction) const
{
	const auto it = list.find(key);
	if (it == list.end()) return false;

	pFunction = it->second;
	return true;
}

