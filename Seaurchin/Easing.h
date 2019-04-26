#pragma once

#include "MoverFunctionExpression.h"

#define SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(name, expression) \
class name ## MoverFunctionExpression : public MoverFunctionExpression \
{ \
public: \
    name ## MoverFunctionExpression() {} \
    double Execute(const MoverFunctionExpressionVariables& var) const override { expression; } \
}

namespace easing
{
	namespace {
		inline double bounce(double begin, double diff, double progress) {
			if ((progress) < (1 / 2.75)) {
				return begin + diff * (7.5625 * progress * progress);
			}
			else if (progress < (2 / 2.75)) {
				progress -= (1.5 / 2.75);
				return begin + diff * (7.5625 * progress * progress + 0.75);
			}
			else if (progress < (2.5 / 2.75)) {
				progress -= (2.25 / 2.75);
				return begin + diff * (7.5625 * progress * progress + 0.9375);
			}
			else {
				progress -= (2.625 / 2.75);
				return begin + diff * (7.5625 * progress * progress + 0.984375);
			}
		}
	}

	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(Linear, return var.Begin + var.Diff * var.Progress);
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InQuad, return var.Begin + var.Diff * var.Progress * var.Progress);
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutQuad, return var.Begin - var.Diff * var.Progress * (var.Progress - 2.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutQuad, return var.Begin + (var.Progress < 0.5) ? var.Diff * var.Progress * var.Progress * 2.0 : (-var.Diff * (var.Progress * (var.Progress - 2.0) - 1.0) / 2.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InCubic, return var.Begin + var.Diff * pow(var.Progress, 3.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutCubic, return var.Begin + var.Diff * (pow(var.Progress - 1.0, 3.0) + 1.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutCubic, return var.Begin + var.Diff / 2.0 * (var.Progress < 0.5) ? pow(2 * var.Progress, 3.0) : (pow(2.0 * var.Progress - 2.0, 3.0) + 2.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InQuartic, return var.Begin + var.Diff * pow(var.Progress, 4.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutQuartic, return var.Begin - var.Diff * (pow(var.Progress - 1.0, 4.0) - 1.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutQuartic, return var.Begin + var.Diff / 2.0 * (var.Progress < 0.5) ? pow(2.0 * var.Progress, 4.0) : (-pow(2.0 * var.Progress - 2.0, 4.0) - 2.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InQuintic, return var.Begin + var.Diff * pow(var.Progress, 5.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutQuintic, return var.Begin - var.Diff * (pow(var.Progress - 1.0, 5.0) + 1.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutQuintic, return var.Begin + var.Diff / 2.0 * (var.Progress < 0.5) ? pow(2.0 * var.Progress, 5.0) : (-pow(2.0 * var.Progress - 2.0, 5.0) + 2.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InSinusoidal, return var.Begin + var.Diff * (1.0 - cos(var.Progress * (M_PI / 2.0))));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutSinusoidal, return var.Begin + var.Diff * sin(var.Progress * (M_PI / 2.0)));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutSinusoidal, return var.Begin + var.Diff / 2.0 * (cos(var.Progress * M_PI) - 1.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InExponential, return var.Begin + var.Diff * pow(2.0, 10.0 * (var.Progress - 1.0)));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutExponential, return var.Begin + var.Diff * (1.0 - pow(2.0, -10.0 * (var.Progress))));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutExponential, return var.Begin + var.Diff / 2.0 * (var.Progress < 0.5) ? pow(2.0, 10.0 * (2.0 * var.Progress - 1.0)) : (2.0 - pow(2.0, -10.0 * var.Progress)));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InCircular, return var.Begin - var.Diff * (sqrt(1.0 - var.Progress * var.Progress) - 1.0));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutCircular, return var.Begin + var.Diff * sqrt(1.0 - (var.Progress - 1.0) * (var.Progress - 1.0)));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutCircular, return var.Begin + var.Diff / 2.0 * (1.0 + (var.Progress < 0.5) ? -sqrt(1.0 - 4.0 * var.Progress * var.Progress) : sqrt(1.0 - 4.0 * (var.Progress - 1.0) * (var.Progress - 1.0))));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InElastic, return var.Begin - (var.Diff * pow(2, 10 * (var.Progress - 1.0)) * sin(((var.Progress - 1.0) / 0.3 - 0.25) * (2.0 * M_PI))));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutElastic, return var.Begin + var.Diff * (1.0 + pow(2, -10 * var.Progress) * sin((var.Progress / 0.3 - 0.25) * (2 * M_PI))));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutElastic, return var.Begin + 0.5 * var.Diff * (var.Progress < 0.5) ? (-pow(2.0, 10.0 * var.Progress) * sin((var.Progress / 0.45 - 0.25) * (2.0 * M_PI))) : (2.0 + pow(2.0, -10 * var.Progress) * sin((var.Progress / 0.45 - 0.25) * (2.0 * M_PI))));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InBack, return var.Begin + var.Diff * var.Progress * var.Progress * (2.70158 * var.Progress - 1.70158));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutBack, return var.Begin + var.Diff * (1.0 + pow(var.Progress - 1.0, 2.0) * (2.70158 * var.Progress - 1.0)));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutBack, return var.Begin + 2.0 * var.Diff * (var.Progress < 0.5) ? (pow(var.Progress, 2.0) * (7.189819 * var.Progress - 2.5949095)) : (1.0 + pow(var.Progress - 1.0, 2.0) * (7.189819 * var.Progress - 4.5949095)));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InBounce, return var.End - bounce(0, var.Diff, 1.0 - var.Progress));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(OutBounce, return bounce(var.Begin, var.Diff, var.Progress));
	SU_DEF_EASING_MOVER_FUNCTION_EXPRESSION(InOutBounce, return var.Begin + 0.5 * (var.Diff + (var.Progress < 0.5) ? -bounce(0, var.Diff, -var.Progress * 2.0 + 1) : bounce(0, var.Diff, var.Progress * 2.0 - 1)));

	bool RegisterDefaultMoverFunctionExpressions();
}
