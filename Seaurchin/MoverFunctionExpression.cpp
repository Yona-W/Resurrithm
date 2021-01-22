#define _USE_MATH_DEFINES

#include "MoverFunctionExpression.h"

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


bool ParseMoverFunctionExpression(MoverFunctionExpressionSharedPtr &root, const std::string &);


MoverFunctionExpressionManager * MoverFunctionExpressionManager::inst;

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

bool MoverFunctionExpressionManager::Register(const std::string &key, const std::string &expression)
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

bool MoverFunctionExpressionManager::Register(const std::string &key, MoverFunctionExpression *pFunction)
{
    const MoverFunctionExpressionSharedPtr ptr(pFunction);
    return Register(key, ptr);
}

bool MoverFunctionExpressionManager::Register(const std::string &key, const MoverFunctionExpressionSharedPtr &pFunction)
{
    if (list.find(key) != list.end()) {
        spdlog::get("main")->warn(u8"\"{0}\" 関数は既に登録されています。", key);
        return false;
    }

    list.insert(std::make_pair(key, pFunction));

    return true;
}

bool MoverFunctionExpressionManager::IsRegistered(const std::string &key)
{
    BOOST_ASSERT(!!inst);
    if (!inst) return false;

    MoverFunctionExpressionSharedPtr pFunction;
    const bool retVal = GetInstance().Find(key, pFunction);
    return retVal && pFunction;
}

bool MoverFunctionExpressionManager::Find(const std::string &key, MoverFunctionExpressionSharedPtr &pFunction) const
{
    const auto it = list.find(key);
    if (it == list.end()) return false;

    pFunction = it->second;
    return true;
}


#define BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

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

namespace parser_impl {
    using namespace boost::spirit;
    namespace phx = boost::phoenix;

    template<typename Iterator>
    struct mover_function_grammer
        : qi::grammar<Iterator, MoverFunctionExpressionSharedPtr(), ascii::space_type>
    {
        qi::rule<Iterator, MoverFunctionExpressionSharedPtr(), ascii::space_type> expr, term, fctr;

        mover_function_grammer() : mover_function_grammer::base_type(expr)
        {
            expr =           term[qi::_val = qi::_1]
                >> *(('+' >> term[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<AddMoverFunctionExpression>, qi::_val, qi::_1)])
                   | ('-' >> term[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<SubMoverFunctionExpression>, qi::_val, qi::_1)]));
            term =           fctr[qi::_val = qi::_1]
                >> *(('*' >> fctr[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<MulMoverFunctionExpression>, qi::_val, qi::_1)])
                   | ('/' >> fctr[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<DivMoverFunctionExpression>, qi::_val, qi::_1)]));
            fctr = qi::double_        [qi::_val = phx::bind(&MakeExp_double<LiteralMoverFunctionExpression>, qi::_1)]
                | (qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = qi::_1]
                | qi::lit("begin")       [qi::_val = phx::bind(&MakeExp<BeginMoverFunctionExpression>)]
                | qi::lit("end")         [qi::_val = phx::bind(&MakeExp<EndMoverFunctionExpression>)]
                | qi::lit("diff")        [qi::_val = phx::bind(&MakeExp<DiffMoverFunctionExpression>)]
                | qi::lit("current")     [qi::_val = phx::bind(&MakeExp<CurrentMoverFunctionExpression>)]
                | qi::lit("progress")    [qi::_val = phx::bind(&MakeExp<ProgressMoverFunctionExpression>)]
                | qi::lit('e')           [qi::_val = phx::bind(&MakeExp<EMoverFunctionExpression>)]
                | qi::lit("log2e")       [qi::_val = phx::bind(&MakeExp<LOG2EMoverFunctionExpression>)]
                | qi::lit("log10e")      [qi::_val = phx::bind(&MakeExp<LOG10EMoverFunctionExpression>)]
                | qi::lit("ln2")         [qi::_val = phx::bind(&MakeExp<LN2MoverFunctionExpression>)]
                | qi::lit("ln10")        [qi::_val = phx::bind(&MakeExp<LN10MoverFunctionExpression>)]
                | qi::lit("pi")          [qi::_val = phx::bind(&MakeExp<PIMoverFunctionExpression>)]
                | qi::lit("pi_2")        [qi::_val = phx::bind(&MakeExp<PI_2MoverFunctionExpression>)]
                | qi::lit("pi_4")        [qi::_val = phx::bind(&MakeExp<PI_4MoverFunctionExpression>)]
                | qi::lit("inv_pi")      [qi::_val = phx::bind(&MakeExp<INV_PIMoverFunctionExpression>)]
                | qi::lit("inv_pi_2")    [qi::_val = phx::bind(&MakeExp<INV_PI_2MoverFunctionExpression>)]
                | qi::lit("inv_sqrtpi_2")[qi::_val = phx::bind(&MakeExp<INV_SQRTPI_2MoverFunctionExpression>)]
                | qi::lit("sqrt2")       [qi::_val = phx::bind(&MakeExp<SQRT2MoverFunctionExpression>)]
                | qi::lit("inv_sqrt2")   [qi::_val = phx::bind(&MakeExp<INV_SQRT2MoverFunctionExpression>)]
                | (qi::lit("abs")   >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<AbsMoverFunctionExpression>,   qi::_1)]
                | (qi::lit("round") >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<RoundMoverFunctionExpression>, qi::_1)]
                | (qi::lit("ceil")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<CeilMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("floor") >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<FloorMoverFunctionExpression>, qi::_1)]
                | (qi::lit("exp")   >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<ExpMoverFunctionExpression>,   qi::_1)]
                | (qi::lit("ln")    >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<LnMoverFunctionExpression>,    qi::_1)]
                | (qi::lit("log")   >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<LogMoverFunctionExpression>,   qi::_1)]
                | (qi::lit("sin")   >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<CosMoverFunctionExpression>,   qi::_1)]
                | (qi::lit("cos")   >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<SinMoverFunctionExpression>,   qi::_1)]
                | (qi::lit("tan")   >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<TanMoverFunctionExpression>,   qi::_1)]
                | (qi::lit("asin")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<AsinMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("acos")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<AcosMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("atan")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<AtanMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("sinh")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<SinhMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("cosh")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<CoshMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("tanh")  >> qi::lit('(') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr<TanhMoverFunctionExpression>,  qi::_1)]
                | (qi::lit("add")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<AddMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("sub")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<SubMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("mul")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<MulMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("div")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<DivMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("mod")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<ModMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("pow")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<PowMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("min")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<MinMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("max")   >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<MaxMoverFunctionExpression>,  qi::_1, qi::_2)]
                | (qi::lit("rand")  >> qi::lit('(') >> expr >> qi::lit(',') >> expr >> qi::lit(')'))[qi::_val = phx::bind(&MakeExp_Ptr_Ptr<RandMoverFunctionExpression>, qi::_1, qi::_2)];
        }
    };

    mover_function_grammer<std::string::const_iterator> gMoverFunc;
}

bool ParseMoverFunctionExpression(MoverFunctionExpressionSharedPtr &root, const std::string &expression)
{
    return boost::spirit::qi::phrase_parse(expression.begin(), expression.end(), parser_impl::gMoverFunc, boost::spirit::ascii::space, root);
}
